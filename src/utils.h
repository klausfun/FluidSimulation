#pragma once

#include <utility>
#include <random>
#include <array>


constexpr std::array<std::pair<int, int>, 4> deltas{{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};

template<typename T>
T get_g() { return 0.1; };

std::mt19937 rnd(1337);

template<typename T>
T random01() {
    return T::from_raw((rnd() & ((1 << 16) - 1)));
}