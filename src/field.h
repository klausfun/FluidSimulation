#pragma once

#include <array>
#include <cassert>
#include <cstring>

#include "fixed.h"
#include "vectorField.h"
#include "utils.h"

template<typename PType, typename VType, typename VFType, int N_val, int K_val>
class Field {
    int N = 0, K = 0;

    char field[N_val][K_val]{};

    VectorField <VType, N_val, K_val> velocity = {};
    VectorField <VFType, N_val, K_val> velocity_flow = {};

    int64_t dirs[N_val][K_val]{};
    int last_use[N_val][K_val]{};
    int UT = 0;

    PType rho[256];
    PType p[N_val][K_val]{}, old_p[N_val][K_val]{};
    VType g = get_g<VType>();

public:
    Field(const std::array<std::string, N_val * K_val> &input_field) : N(N_val), K(K_val) {
        for (size_t i = 0; i < N; ++i) {
            strncpy(field[i], input_field[i].c_str(), K);
            field[i][K] = '\0';
        }
        init();
    }

    void next(int i) {
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
    }

private:
    void init() {
        rho[' '] = 0.01;
        rho['.'] = int64_t(1000);
        for (int x = 0; x < N; ++x) {
            for (int y = 0; y < K; ++y) {
                if (field[x][y] == '#')
                    continue;
                for (auto [dx, dy]: deltas) {
                    dirs[x][y] += (field[x + dx][y + dy] != '#');
                }
            }
        }
    }

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
    }

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
    }

    void swap(int x1, int y1, int x2, int y2) {
        std::swap(field[x1][y1], field[x2][y2]);
        std::swap(p[x1][y1], p[x2][y2]);
        std::swap(velocity.v[x1][y1], velocity.v[x2][y2]);
    }

    std::tuple<VFType, bool, std::pair<int, int>> propagate_flow(int x, int y, VFType lim) {
        last_use[x][y] = UT - 1;
        VFType ret{};
        for (auto [dx, dy]: deltas) {
            int nx = x + dx, ny = y + dy;
            if (field[nx][ny] != '#' && last_use[nx][ny] < UT) {
                VType cap = velocity.get(x, y, dx, dy);
                VFType flow = velocity_flow.get(x, y, dx, dy);
                if (fabs(flow - VFType(cap)) <= 0.0001) {
                    continue;
                }
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
    }

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
    }

    void apply_external_forces() {
        for (size_t x = 0; x < N; ++x) {
            for (size_t y = 0; y < K; ++y) {
                if (field[x][y] == '#')
                    continue;
                if (field[x + 1][y] != '#')
                    velocity.add(x, y, 1, 0, g);
            }
        }
    }

    void apply_forces_from_p(PType &total_delta_p) {
        memcpy(old_p, p, sizeof(p));
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
                        p[x][y] -= force / dirs[x][y];
                        total_delta_p -= force / dirs[x][y];
                    }
                }
            }
        }
    }

    void make_flow_from_velocities() {
        velocity_flow = {};
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
    }

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
                            force *= 0.8;
                        if (field[x + dx][y + dy] == '#') {
                            p[x][y] += force / dirs[x][y];
                            total_delta_p += force / dirs[x][y];
                        } else {
                            p[x + dx][y + dy] += force / dirs[x + dx][y + dy];
                            total_delta_p += force / dirs[x + dx][y + dy];
                        }
                    }
                }
            }
        }
    }

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
    }
};