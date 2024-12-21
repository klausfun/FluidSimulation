#pragma once

#include <cstdint>
#include "fixedBase.h"

template <size_t N>
struct nSizeType;

template<> struct nSizeType<8>  {using type = int8_t; };
template<> struct nSizeType<16> {using type = int16_t;};
template<> struct nSizeType<32> {using type = int32_t;};
template<> struct nSizeType<64> {using type = int64_t;};

template <size_t N, size_t K> requires (N >= K)
using Fixed = FixedBase<typename nSizeType<N>::type, K>;

template <size_t N, size_t K>
struct FastFixedWrap: FastFixedWrap<N+1, K> {using type = typename FastFixedWrap<N+1, K>::type;};

template<size_t K> struct FastFixedWrap<8, K>:  Fixed<8, K>  {using type = Fixed<8, K>;};
template<size_t K> struct FastFixedWrap<16, K>: Fixed<16, K> {using type = Fixed<16, K>;};
template<size_t K> struct FastFixedWrap<32, K>: Fixed<32, K> {using type = Fixed<32, K>;};
template<size_t K> struct FastFixedWrap<64, K>: Fixed<64, K> {using type = Fixed<64, K>;};

template <size_t N, size_t K>
using FastFixed = typename FastFixedWrap<N, K>::type;
