
#pragma once

#include "fixed.h"
#include "fastFixed.h"
#include <array>

#include "typesAndField.h"

using std::array;

#ifndef SIZES
#define SIZES
#endif

constexpr array t{TYPES};
constexpr array s{DYNAMIC, SIZES};

template <int num>
using type = std::conditional_t<
        num == FLOAT, float,
        std::conditional_t<
                num == DOUBLE, double,
                std::conditional_t<
                        num < 10000, Fixed<num/100, num%100>,
                        FastFixed<num/10000, num%10000>
                >
        >
>;

template <typename P, typename V, typename VF, size_t N, size_t M>
std::unique_ptr<AbstractField> generateSim() {
    return std::make_unique<Field<P, V, VF, N, M>>();
}

template <int index>
constexpr auto simulatorsGenerator() {
    auto res = simulatorsGenerator<index+1>();
    res[index] = generateSim<type<t[index/(t.size()*t.size()*s.size())]>, type<t[index%(t.size()*t.size()*s.size())/(t.size()*s.size())]>, type<t[index%(t.size()*s.size())/s.size()]>, s[index%s.size()].first, s[index%s.size()].second>;
    return res;
}

using genfunc = std::unique_ptr<AbstractField>(*)();

template <>
constexpr auto simulatorsGenerator<t.size()*t.size()*t.size()*s.size()>() {
    return array<genfunc, t.size()*t.size()*t.size()*s.size()>();
}

template <int index>
constexpr auto typesGenerator() {
    auto res = typesGenerator<index+1>();
    res[index] = {t[index/(t.size()*t.size()*s.size())], t[index%(t.size()*t.size()*s.size())/(t.size()*s.size())], t[index%(t.size()*s.size())/s.size()], s[index%s.size()].first, s[index%s.size()].second};
    return res;
}

template <>
constexpr auto typesGenerator<t.size()*t.size()*t.size()*s.size()>() {
    return array<tuple<int, int, int, size_t, size_t>, t.size()*t.size()*t.size()*s.size()>();
}

constexpr auto generateTypes = typesGenerator<0>;
constexpr auto generateSimulators = simulatorsGenerator<0>;