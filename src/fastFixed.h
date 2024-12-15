#pragma once

#include <iostream>
#include <array>
#include <cassert>
#include <random>
#include <tuple>
#include <algorithm>
#include <cstring>
#include <limits>

#include "fixedBase.h"

template<int N, int K> requires(K >= 0)
struct Fixed;

template<int N, int K> requires(K >= 0)
struct FastFixed : public FixedBase<N, K,
        typename std::conditional_t<
                (N <= 8), int_fast8_t,
                typename std::conditional_t<
                        (N <= 16), int_fast16_t,
                        typename std::conditional_t<
                                (N <= 32), int_fast32_t,
                                typename std::conditional_t<
                                        (N <= 64), int_fast64_t,
                                        void>>>>,
        FastFixed<N, K>> {

    using Base = FixedBase<N, K, typename std::conditional_t<
            (N <= 8), int_fast8_t,
            typename std::conditional_t<
                    (N <= 16), int_fast16_t,
                    typename std::conditional_t<
                            (N <= 32), int_fast32_t,
                            typename std::conditional_t<
                                    (N <= 64), int_fast64_t,
                                    void>>>>, FastFixed<N, K>>;
    using Base::Base;

    template<int N2, int K2>
    constexpr FastFixed(const Fixed<N2, K2>& other)
            : Base(other) {}

    template<int N2, int K2>
    constexpr FastFixed(const FastFixed<N2, K2>& other)
            : Base(other) {}


    auto operator<=>(const FastFixed& other) const {
        return this->v <=> other.v;
    }

    bool operator==(const FastFixed& other) const {
        return this->v == other.v;
    }
};
