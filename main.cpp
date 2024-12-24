#include <memory>
#include <csignal>
#include <nlohmann/json.hpp>

#include "src/field.h"
#include "src/parser.h"
#include "src/typesAndField.h"

auto simulators = generateSimulators();
auto types = generateTypes();

using json = nlohmann::json;

bool save = false;

void handler(int x) {
    save = true;
}

int main(int argc, char* argv[]) {
    signal(SIGINT, handler);

    Parser parser{};
    parser.parseArgs(argc, argv);
    std::cout << parser.p_type << parser.v_type << parser.vf_type << std::endl;

    FieldConfig info(parser.input_filename);

    tuple need = {parser.p_type, parser.v_type, parser.vf_type, info.h, info.w};
    auto index = std::find(types.begin(), types.end(), need) - types.begin();
    if (index == types.size()) {
        need = {parser.p_type, parser.v_type, parser.vf_type, 0, 0};
        index = std::find(types.begin(), types.end(), need) - types.begin();
    }
    if (index == types.size()) {
        std::cout << "Simulator with chosen types does not exist\n";
        exit(EXIT_FAILURE);
    }

//    auto& field = simulators[index];
    auto field = simulators[index]();
    field->init(info, parser);

    for (size_t i = info.tick; i < 1000000; ++i) {
        if (save) {
            field->save(parser.output_filename, i);

            std::cout << "Enter any number to continue: ";

            save = false;
            int tmp;
            std::cin >> tmp;

            std::cout << std::endl;
        }

        field->nextTick(i);
    }
}