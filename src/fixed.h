#pragma once

#include "fixedBase.h"

template<int N>
struct real_fixed_type {
    using fixed_type = std::conditional_t<N == 8, int8_t,
            std::conditional_t<N == 16, int16_t,
                    std::conditional_t<N == 32, int32_t,
                            std::conditional_t<N == 64, int64_t, void>>>>;
};

template <size_t N, size_t K> requires (N >= K)
using Fixed = FixedBase<typename real_fixed_type<N>::fixed_type, K>;