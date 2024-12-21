#include <memory>
#include <csignal>
#include <nlohmann/json.hpp>

#include "src/field.h"
#include "src/parser.h"
#include "src/typeGeneration.h"

constexpr auto simulators = generateSimulators();
constexpr auto types = generateTypes();
using json = nlohmann::json;

bool save = false;

void handler(int x) {
    save = true;
}

int main(int argc, char* argv[]) {
    signal(SIGINT, handler);

    SimSetts st = parseArgs(argc, argv);
    FieldConfig info(st.input_filename);

    tuple need = {st.p_type, st.v_type, st.vf_type, info.h, info.w};
    auto index = std::find(types.begin(), types.end(), need) - types.begin();
    if (index == types.size()) {
        need = {st.p_type, st.v_type, st.vf_type, 0, 0};
        index = std::find(types.begin(), types.end(), need) - types.begin();
    }
    if (index == types.size()) {
        std::cout << "Simulator with chosen types does not exist\n";
        exit(EXIT_FAILURE);
    }

    auto field = simulators[index]();
    field->init(info, st);

    for (size_t i = info.tick; i < 1000000; ++i) {
        if (save) {
            field->save(st.output_filename, i);

            std::cout << "Enter any number to continue: ";

            save = false;
            int tmp;
            std::cin >> tmp;

            std::cout << std::endl;
        }

        field->nextTick(i);
    }
}