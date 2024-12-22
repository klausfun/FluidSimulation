#pragma once

#include <random>
#include <algorithm>
#include <cstring>
#include <fstream>

#include "utils.h"
#include "vectorField.h"
#include "wrapperArray.h"
#include "parser.h"

using std::tuple, std::pair, std::ofstream;
using json = nlohmann::json;

struct FieldConfig {
    double rhoFluid{};
    double rhoField{};
    double g{};
    size_t h{};
    size_t w{};
    size_t tick{};
    std::vector<std::string> field;

    explicit FieldConfig(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Unable to open file: " + filename);
        }

        json config_json;
        file >> config_json;

        rhoField = config_json.at("rhoField").get<double>();
        rhoFluid = config_json.at("rhoFluid").get<double>();
        g = config_json.at("g").get<double>();
        h = config_json.at("N").get<size_t>();
        w = config_json.at("K").get<size_t>();
        tick = config_json.at("Tick").get<size_t>();
        field = config_json.at("field").get<std::vector<std::string>>();

        if (field.size() != h) {
            throw std::runtime_error("Field row count mismatch: expected " +
                                     std::to_string(h) + ", got " +
                                     std::to_string(field.size()));
        }
        for (const auto &row: field) {
            if (row.size() != w) {
                throw std::runtime_error("Field column size mismatch: expected " +
                                         std::to_string(w) + ", got " +
                                         std::to_string(row.size()));
            }
        }
    };
};

struct AbstractField {
    virtual void nextTick(int i) = 0;
    virtual void init(const FieldConfig& f, const Parser& parser) = 0;
    virtual void save(const std::string& filename, size_t i) = 0;
    virtual ~AbstractField() = default;
};

template<typename PType, typename VType, typename VFType, int N_val, int K_val>
struct Field final: AbstractField {
    int64_t n_ticks{}, cur_tick{}; std::string out_name;

    int N = 0, K = 0;

    Array<char, N_val, K_val> field{};

    VectorField <VType, N_val, K_val> velocity = {};
    VectorField <VFType, N_val, K_val> velocity_flow = {};

    Array<int64_t, N_val, K_val> last_use{}, dirs{};
    int UT = 0;

    PType rho[256];
    Array<PType, N_val, K_val> p{}, old_p{};
    VType g{};
    std::mt19937 rnd;

    Field(): rnd(1337) {}

    void nextTick(int i) override {
        PType total_delta_p = int64_t(0);

        apply_external_forces();

        apply_forces_from_p(total_delta_p);

        make_flow_from_velocities();

        recalculate_p(total_delta_p);

        if (apply_move_on_flow()) {
            std::cout << "Tick " << i << ":\n";
            for (int j = 0; j < N; j++) {
                for (int k = 0; k < K; k++) {
                    std::cout << field[j][k];
                }
                std::cout << "\n";
            }
            std::cout.flush();
        }

        if (!out_name.empty() && (++cur_tick == n_ticks)) {
            ofstream out(out_name);

            auto cnt = std::count_if(rho, rho+256, [](auto i){return i!=0l;});

            out << N << " " << K << " " << g << " " << cnt << "\n";
            for (int i = 0; i < 256; i++) {
                if (rho[i] == 0l) continue;
                out << ((uint8_t)i) << " " << rho[i] << "\n";
            }

            for (int x = 0; x < N; x++)
            {
                for (int y = 0; y < K; y++)
                {
                    out << field[x][y];
                }
                out << "\n";
            }
            out.close();

            cur_tick = 0;
        }
    };

    void init(const FieldConfig& f, const Parser& parser) override {
        g = f.g; N = f.h; K = f.w;
        rho[' '] = f.rhoField;
        rho['.'] = f.rhoFluid;

        velocity.init(N, K);
        velocity_flow.init(N, K);
        p.init(N, K); old_p.init(N, K);
        last_use.init(N, K); dirs.init(N, K);
        field.init(N, K);

        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < K; j++) {
                field[i][j] = f.field[i][j];
            }
        }

        n_ticks = parser.n_ticks;
        out_name = parser.output_filename;

        for (size_t x = 0; x < N; ++x) {
            for (size_t y = 0; y < K; ++y) {
                if (field[x][y] == '#')
                    continue;
                for (auto [dx, dy] : deltas) {
                    dirs[x][y] += (field[x + dx][y + dy] != '#');
                }
            }
        }
    };

    void propagate_stop(int x, int y, bool force = false) {
        if (!force) {
            bool stop = true;
            for (auto [dx, dy]: deltas) {
                int nx = x + dx, ny = y + dy;
                if (field[nx][ny] != '#' && last_use[nx][ny] < UT - 1 && velocity.get(x, y, dx, dy) > int64_t(0)) {
                    stop = false;
                    break;
                }
            }
            if (!stop) {
                return;
            }
        }
        last_use[x][y] = UT;
        for (auto [dx, dy]: deltas) {
            int nx = x + dx, ny = y + dy;
            if (field[nx][ny] == '#' || last_use[nx][ny] == UT || velocity.get(x, y, dx, dy) > int64_t(0)) {
                continue;
            }
            propagate_stop(nx, ny);
        }
    };

    VType move_prob(int x, int y) {
        VType sum;
        for (size_t i = 0; i < deltas.size(); ++i) {
            auto [dx, dy] = deltas[i];
            int nx = x + dx, ny = y + dy;
            if (field[nx][ny] == '#' || last_use[nx][ny] == UT) {
                continue;
            }

            VType v = velocity.get(x, y, dx, dy);
            if (v < int64_t(0)) {
                continue;
            }

            sum += v;
        }
        return sum;
    };

    void swap(int x1, int y1, int x2, int y2) {
        std::swap(field[x1][y1], field[x2][y2]);
        std::swap(p[x1][y1], p[x2][y2]);
        std::swap(velocity.v[x1][y1], velocity.v[x2][y2]);
    };

    std::tuple<VFType, bool, std::pair<int, int>> propagate_flow(int x, int y, VFType lim) {
        last_use[x][y] = UT - 1;
        VFType ret{};
        for (auto [dx, dy]: deltas) {
            int nx = x + dx, ny = y + dy;
            if (field[nx][ny] != '#' && last_use[nx][ny] < UT) {
                VType cap = velocity.get(x, y, dx, dy);
                VFType flow = velocity_flow.get(x, y, dx, dy);
                if (fabs(double(flow - VFType(cap))) <= 0.0001) continue;
                // assert(v >= velocity_flow.get(x, y, dx, dy));
                VFType vp = std::min(lim, VFType(cap) - flow);
                if (last_use[nx][ny] == UT - 1) {
                    velocity_flow.add(x, y, dx, dy, vp);
                    last_use[x][y] = UT;
                    // cerr << x << " " << y << " -> " << nx << " " << ny << " " << vp << " / " << lim << "\n";
                    return {vp, 1, {nx, ny}};
                }
                auto [t, prop, end] = propagate_flow(nx, ny, vp);
                ret += t;
                if (prop) {
                    velocity_flow.add(x, y, dx, dy, t);
                    last_use[x][y] = UT;
                    // cerr << x << " " << y << " -> " << nx << " " << ny << " " << t << " / " << lim << "\n";
                    return {t, end != std::pair(x, y), end};
                }
            }
        }
        last_use[x][y] = UT;
        return {ret, 0, {0, 0}};
    };

    bool propagate_move(int x, int y, bool is_first) {
        last_use[x][y] = UT - is_first;
        bool ret = false;
        int nx = -1, ny = -1;
        do {
            std::array<VType, deltas.size()> tres;
            VType sum;
            for (size_t i = 0; i < deltas.size(); ++i) {
                auto [dx, dy] = deltas[i];
                int nx = x + dx, ny = y + dy;
                if (field[nx][ny] == '#' || last_use[nx][ny] == UT) {
                    tres[i] = sum;
                    continue;
                }
                VType v = velocity.get(x, y, dx, dy);
                if (v < int64_t(0)) {
                    tres[i] = sum;
                    continue;
                }
                sum += v;
                tres[i] = sum;
            }

            if (sum == int64_t(0)) {
                break;
            }

            VType p = random01<VType>() * sum;
            size_t d = std::ranges::upper_bound(tres, p) - tres.begin();

            auto [dx, dy] = deltas[d];
            nx = x + dx;
            ny = y + dy;
            assert(velocity.get(x, y, dx, dy) > int64_t(0) && field[nx][ny] != '#' && last_use[nx][ny] < UT);

            ret = (last_use[nx][ny] == UT - 1 || propagate_move(nx, ny, false));
        } while (!ret);
        last_use[x][y] = UT;
        for (size_t i = 0; i < deltas.size(); ++i) {
            auto [dx, dy] = deltas[i];
            int nx = x + dx, ny = y + dy;
            if (field[nx][ny] != '#' && last_use[nx][ny] < UT - 1 && velocity.get(x, y, dx, dy) < int64_t(0)) {
                propagate_stop(nx, ny);
            }
        }
        if (ret) {
            if (!is_first) {
                swap(x, y, nx, ny);
            }
        }
        return ret;
    };

    void apply_external_forces() {
        for (size_t x = 0; x < N; ++x) {
            for (size_t y = 0; y < K; ++y) {
                if (field[x][y] == '#')
                    continue;
                if (field[x + 1][y] != '#')
                    velocity.add(x, y, 1, 0, g);
            }
        }
    };

    void apply_forces_from_p(PType &total_delta_p) {
        old_p = p;
        for (size_t x = 0; x < N; ++x) {
            for (size_t y = 0; y < K; ++y) {
                if (field[x][y] == '#')
                    continue;
                for (auto [dx, dy]: deltas) {
                    int nx = x + dx, ny = y + dy;
                    if (field[nx][ny] != '#' && old_p[nx][ny] < old_p[x][y]) {
                        PType delta_p = old_p[x][y] - old_p[nx][ny];
                        PType force = delta_p;
                        VType &contr = velocity.get(nx, ny, -dx, -dy);
                        if (PType(contr) * rho[(int) field[nx][ny]] >= force) {
                            contr -= VType(force / rho[(int) field[nx][ny]]);
                            continue;
                        }
                        force -= PType(contr) * rho[(int) field[nx][ny]];
                        contr = int64_t(0);
                        velocity.add(x, y, dx, dy, VType(force / rho[(int) field[x][y]]));
                        p[x][y] -= force / PType(dirs[x][y]);
                        total_delta_p -= force / PType(dirs[x][y]);
                    }
                }
            }
        }
    };

    void make_flow_from_velocities() {
        velocity_flow.clear();

        bool prop = false;
        do {
            UT += 2;
            prop = false;
            for (size_t x = 0; x < N; ++x) {
                for (size_t y = 0; y < K; ++y) {
                    if (field[x][y] != '#' && last_use[x][y] != UT) {
                        auto [t, local_prop, _] = propagate_flow(x, y, int64_t(1));
                        if (t > int64_t(0)) {
                            prop = true;
                        }
                    }
                }
            }
        } while (prop);
    };

    void recalculate_p(PType &total_delta_p) {
        for (size_t x = 0; x < N; ++x) {
            for (size_t y = 0; y < K; ++y) {
                if (field[x][y] == '#')
                    continue;
                for (auto [dx, dy]: deltas) {
                    VType old_v = velocity.get(x, y, dx, dy);
                    VFType new_v = velocity_flow.get(x, y, dx, dy);
                    if (old_v > int64_t(0)) {
                        assert(VType(new_v) <= old_v);
                        velocity.get(x, y, dx, dy) = VType(new_v);
                        auto force = PType(old_v - VType(new_v)) * rho[(int) field[x][y]];
                        if (field[x][y] == '.')
                            force *= PType(0.8);
                        if (field[x + dx][y + dy] == '#') {
                            p[x][y] += force / PType(dirs[x][y]);
                            total_delta_p += force / PType(dirs[x][y]);
                        } else {
                            p[x + dx][y + dy] += force / PType(dirs[x + dx][y + dy]);
                            total_delta_p += force / PType(dirs[x + dx][y + dy]);
                        }
                    }
                }
            }
        }
    };

    bool apply_move_on_flow() {
        UT += 2;
        bool prop = false;
        for (size_t x = 0; x < N; ++x) {
            for (size_t y = 0; y < K; ++y) {
                if (field[x][y] != '#' && last_use[x][y] != UT) {
                    if (random01<VType>() < move_prob(x, y)) {
                        prop = true;
                        propagate_move(x, y, true);
                    } else {
                        propagate_stop(x, y, true);
                    }
                }
            }
        }

        return prop;
    };

    ~Field() override = default;

    void save(const std::string& filename, size_t i) override {
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Unable to open file: " + filename);
        }

        json config_json;
        config_json["rhoField"] = double(rho[' ']);
        config_json["rhoFluid"] = double(rho['.']);
        config_json["g"] = double(g);
        config_json["N"] = int(N);
        config_json["K"] = int(K);
        config_json["Tick"] = int(i);

        std::vector<std::string> vec(N, std::string(K, ' '));
        for (int y = 0; y < N; y++) {
            for (int x = 0; x < K; x++) {
                vec[y][x] = char(field[y][x]);
            }
        }
        config_json["field"] = vec;

        file << config_json.dump(4);
    };
};
