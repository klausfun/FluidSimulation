
#pragma once

#include "fixedBase.h"

template<int N>
struct real_fast_fixed_type {
    using fast_fixed_type = std::conditional_t<N <= 8, int_fast8_t,
            std::conditional_t<N <= 16, int_fast16_t,
                    std::conditional_t<N <= 32, int_fast32_t,
                            std::conditional_t<N <= 64, int_fast64_t, void>>>>;
};

template <size_t N, size_t K>
using FastFixed = FixedBase<typename real_fast_fixed_type<N>::fast_fixed_type, K>;
