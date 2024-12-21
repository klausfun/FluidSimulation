#pragma once

#include <iostream>
#include <cstddef>

template <typename V, size_t K1>
struct FixedBase {
    V v;
    static const size_t k = K1;

    constexpr FixedBase(int64_t v): v(v << K1) {}
    constexpr FixedBase(float f): v(f * (1ll << K1)) {}
    constexpr FixedBase(double f): v(f * (1ll << K1)) {}
    constexpr FixedBase(): v(0) {}

    template <typename V2, size_t K2>
    constexpr FixedBase(FixedBase<V2, K2> f) {
        if (K1 >= K2) {
            v = (int64_t)f.v << (K1 - K2);
        } else {
            v = (int64_t)f.v >> (K2 - K1);
        }
    }

    static constexpr FixedBase from_raw(int64_t x) {
        FixedBase ret;
        ret.v = x;
        return ret;
    }

    auto operator<=>(const FixedBase&) const = default;
    bool operator== (const FixedBase&) const = default;

    explicit constexpr operator float() const {return (float)v / ((float) (1ll << K1));}
    explicit constexpr operator double() const {return (double)v / ((double) (1ll << K1));}
};

template<typename V1, size_t K1, typename V2, size_t K2>
auto operator+(FixedBase<V1, K1> a, const FixedBase<V2, K2>& b) {
    return FixedBase<V1, K1>::from_raw((K1 > K2) ? (a.v + (b.v << (K1 - K2))) : (a.v + (b.v >> (K2 - K1))));
}

template<typename V1, size_t K1, typename V2, size_t K2>
auto operator-(FixedBase<V1, K1> a, const FixedBase<V2, K2>& b){
    return FixedBase<V1, K1>::from_raw((K1 > K2) ? (a.v - (b.v << (K1 - K2))) : (a.v - (b.v >> (K2 - K1))));
}

#ifdef __SIZEOF_INT128__
template<typename V1, size_t K1, typename V2, size_t K2>
FixedBase<V1, K1> operator*(FixedBase<V1, K1> a, const FixedBase<V2, K2>& b){
    return FixedBase<V1, K1>::from_raw(((__int128_t)a.v * b.v) >> K2);
}

template<typename V1, size_t K1, typename V2, size_t K2>
auto operator/(FixedBase<V1, K1> a, const FixedBase<V2, K2>& b){
    return FixedBase<V1, K1>::from_raw((((__int128_t)a.v << K2) / b.v));
}
#else
template<typename V1, size_t K1, typename V2, size_t K2>
FixedImpl<V1, K1> operator*(FixedImpl<V1, K1> a, const FixedImpl<V2, K2>& b){
    return FixedImpl<V1, K1>(double(a) * double(b));
}

template<typename V1, size_t K1, typename V2, size_t K2>
auto operator/(FixedImpl<V1, K1> a, const FixedImpl<V2, K2>& b){
    return FixedImpl<V1, K1>(double(a) / double(b));
}
#endif

template<typename V1, size_t K1, typename V2, size_t K2>
FixedBase<V1, K1>& operator+=(FixedBase<V1, K1> &a, const FixedBase<V2, K2>& b) {
    return a = a + b;
}

template<typename V1, size_t K1, typename V2, size_t K2>
auto& operator-=(FixedBase<V1, K1> &a, const FixedBase<V2, K2>& b) {
    return a = a - b;
}

template<typename V1, size_t K1, typename V2, size_t K2>
auto& operator*=(FixedBase<V1, K1> &a, const FixedBase<V2, K2>& b) {
    return a = a * b;
}

template<typename V1, size_t K1, typename V2, size_t K2>
auto& operator/=(FixedBase<V1, K1> &a, const FixedBase<V2, K2>& b) {
    return a = a / b;
}

template<typename V1, size_t K1>
auto operator-(FixedBase<V1, K1> x) {
    return FixedBase<V1, K1>::from_raw(-x.v);
}

template<typename V1, size_t K1>
auto abs(FixedBase<V1, K1> x) {
    if (x.v < 0) {
        x.v = -x.v;
    }
    return x;
}

template<typename V1, size_t K1>
std::ostream &operator<<(std::ostream &out, const FixedBase<V1, K1>& x) {
    return out << x.v / (double) (1ll << K1);
}