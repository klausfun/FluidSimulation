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
#include "fastFixed.h"

template<int N, int K> requires(K >= 0)
struct Fixed : public FixedBase<N, K,
        typename std::conditional_t<
        (N == 8), int8_t,
        typename std::conditional_t<
                (N == 16), int16_t,
                typename std::conditional_t<
                        (N == 32), int32_t,
                        typename std::conditional_t<
                                (N == 64), int64_t,
                                void>>>>,
                                Fixed<N, K>> {

    using Base = FixedBase<N, K, typename std::conditional_t<
            (N == 8), int8_t,
            typename std::conditional_t<
                    (N == 16), int16_t,
                    typename std::conditional_t<
                            (N == 32), int32_t,
                            typename std::conditional_t<
                                    (N == 64), int64_t,
                                    void>>>>, Fixed<N, K>>;
    using Base::Base;

    template<int N2, int K2>
    constexpr Fixed(const FastFixed<N2, K2>& other)
            : Base(other) {}

    template<int N2, int K2>
    constexpr Fixed(const Fixed<N2, K2>& other)
            : Base(other) {}

    auto operator<=>(const Fixed& other) const {
        return this->v <=> other.v;
    }

    bool operator==(const Fixed& other) const {
        return this->v == other.v;
    }


};
