#pragma once

#include <iostream>
#include <array>
#include <cassert>
#include <random>
#include <tuple>
#include <algorithm>
#include <cstring>
#include <limits>

#include "utils.h"

template<int N, int K> requires(K >= 0)
struct Fixed {
    int64_t v;

    constexpr Fixed(int64_t v): v(v << K) {}
    constexpr Fixed(float f): v(f * (1LL << K)) {}
    constexpr Fixed(double f): v(f * (1LL << K)) {}
    constexpr Fixed(): v(0) {}

//    explicit constexpr operator float() { return float(v) / (1LL << K); }
//    explicit constexpr operator double() { return double(v) / (1LL << K); }

    static constexpr Fixed from_raw(int64_t x) {
        Fixed ret;
        ret.v = x;
        return ret;
    }

    auto operator<=>(const Fixed&) const = default;
    bool operator==(const Fixed&) const = default;

//    static constexpr Fixed inf = Fixed::from_raw(std::numeric_limits<int32_t>::max());
//    static constexpr Fixed eps = Fixed::from_raw(deltas.size());

    friend Fixed operator+(Fixed a, Fixed b) {
        return Fixed::from_raw(a.v + b.v);
    }

    friend Fixed operator-(Fixed a, Fixed b) {
        return Fixed::from_raw(a.v - b.v);
    }

    friend Fixed operator*(Fixed a, Fixed b) {
        return Fixed::from_raw(((int64_t) a.v * b.v) >> K);
    }

    friend Fixed operator/(Fixed a, Fixed b) {
        return Fixed::from_raw(((int64_t) a.v << K) / b.v);
    }

    friend Fixed &operator+=(Fixed &a, Fixed b) {
        return a = a + b;
    }

    friend Fixed &operator-=(Fixed &a, Fixed b) {
        return a = a - b;
    }

    friend Fixed &operator*=(Fixed &a, Fixed b) {
        return a = a * b;
    }

    friend Fixed &operator/=(Fixed &a, Fixed b) {
        return a = a / b;
    }

    friend Fixed operator-(Fixed x) {
        return Fixed::from_raw(-x.v);
    }

    friend Fixed abs(Fixed x) {
        if (x.v < 0) {
            x.v = -x.v;
        }
        return x;
    }

    friend std::ostream &operator<<(std::ostream &out, Fixed x) {
        return out << x.v / (double) (1 << K);
    }
};

