#pragma once

#include <iostream>
#include <cassert>
#include <limits>
#include <type_traits>

template<int N, int K, typename RealType, typename Type>
class FixedBase {
public:
    static const int n = N, k = K;
    RealType v;

    constexpr FixedBase(int64_t value): v(static_cast<RealType>(value << K)) {}
    constexpr FixedBase(float f): v(static_cast<RealType>(f * (1LL << K))) {}
    constexpr FixedBase(double f): v(static_cast<RealType>(f * (1LL << K))) {}
    constexpr FixedBase(): v(0) {}

    template<int N2, int K2, typename RealType2, typename Type2>
    constexpr FixedBase(const FixedBase<N2, K2, RealType2, Type2>& other) {
        if constexpr (K >= K2) {
            v = static_cast<RealType>(other.v) << (K - K2);
        } else {
            v = static_cast<RealType>(other.v) >> (K2 - K);
        }
    }

    explicit constexpr operator float() const { return static_cast<float>(v) / (1LL << K); }
    explicit constexpr operator double() const { return static_cast<double>(v) / (1LL << K); }

    static constexpr Type from_raw(int64_t x) {
        Type ret;
        ret.v = x;
        return ret;
    }

    constexpr RealType raw_value() const { return v; }

    friend Type operator+(Type a, Type b) {
        return Type::from_raw(a.v + b.v);
    }

    friend Type operator-(Type a, Type b) {
        return Type::from_raw(a.v - b.v);
    }

    friend Type operator*(Type a, Type b) {
        return Type::from_raw(((int64_t) a.v * b.v) >> K);
    }

    friend Type operator/(Type a, Type b) {
        return Type::from_raw(((int64_t)a.v << K) / b.v);
    }

    friend Type& operator+=(Type& a, Type b) {
        return a = a + b;
    }

    friend Type& operator-=(Type& a, Type b) {
        return a = a - b;
    }

    friend Type& operator*=(Type& a, Type b) {
        return a = a * b;
    }

    friend Type& operator/=(Type& a, Type b) {
        return a = a / b;
    }

    friend Type operator-(Type x) {
        return Type::from_raw(-x.v);
    }

    friend Type abs(Type x) {
        if (x.v < 0) {
            x.v = -x.v;
        }
        return x;
    }

    friend std::ostream& operator<<(std::ostream& out, Type x) {
        return out << x.v / (double) (1 << K);
    }
};
