
#pragma once

#include <iostream>
#include <cstddef>

template<typename RealType, size_t K>
class FixedBase {
public:
    RealType v;
    static const size_t k = K;

    constexpr FixedBase(int64_t v): v(v << K) {}
    constexpr FixedBase(float f): v(f * (1ll << K)) {}
    constexpr FixedBase(double f): v(f * (1ll << K)) {}
    constexpr FixedBase(): v(0) {}

    template <typename RealType2, size_t K2>
    constexpr FixedBase(FixedBase<RealType2, K2> f) {
        if (K >= K2) {
            v = (int64_t)f.v << (K - K2);
        } else {
            v = (int64_t)f.v >> (K2 - K);
        }
    }

    static constexpr FixedBase from_raw(int64_t x) {
        FixedBase ret;
        ret.v = x;
        return ret;
    }

    auto operator<=>(const FixedBase&) const = default;
    bool operator== (const FixedBase&) const = default;

    explicit constexpr operator float() const {return (float)v / ((float) (1ll << K));}
    explicit constexpr operator double() const {return (double)v / ((double) (1ll << K));}
};

template<typename V1, size_t K, typename V2, size_t K2>
auto operator+(FixedBase<V1, K> a, const FixedBase<V2, K2>& b) {
    return FixedBase<V1, K>::from_raw((K > K2) ? (a.v + (b.v << (K - K2))) : (a.v + (b.v >> (K2 - K))));
}

template<typename V1, size_t K, typename V2, size_t K2>
auto operator-(FixedBase<V1, K> a, const FixedBase<V2, K2>& b){
    return FixedBase<V1, K>::from_raw((K > K2) ? (a.v - (b.v << (K - K2))) : (a.v - (b.v >> (K2 - K))));
}

#ifdef __SIZEOF_INT128__
template<typename V1, size_t K, typename V2, size_t K2>
FixedBase<V1, K> operator*(FixedBase<V1, K> a, const FixedBase<V2, K2>& b){
    return FixedBase<V1, K>::from_raw(((__int128_t)a.v * b.v) >> K2);
}

template<typename V1, size_t K, typename V2, size_t K2>
auto operator/(FixedBase<V1, K> a, const FixedBase<V2, K2>& b){
    return FixedBase<V1, K>::from_raw((((__int128_t)a.v << K2) / b.v));
}
#else
template<typename V1, size_t K, typename V2, size_t K2>
FixedImpl<V1, K> operator*(FixedImpl<V1, K> a, const FixedImpl<V2, K2>& b){
    return FixedImpl<V1, K>(double(a) * double(b));
}

template<typename V1, size_t K, typename V2, size_t K2>
auto operator/(FixedImpl<V1, K> a, const FixedImpl<V2, K2>& b){
    return FixedImpl<V1, K>(double(a) / double(b));
}
#endif

template<typename V1, size_t K, typename V2, size_t K2>
FixedBase<V1, K>& operator+=(FixedBase<V1, K> &a, const FixedBase<V2, K2>& b) {
    return a = a + b;
}

template<typename V1, size_t K, typename V2, size_t K2>
auto& operator-=(FixedBase<V1, K> &a, const FixedBase<V2, K2>& b) {
    return a = a - b;
}

template<typename V1, size_t K, typename V2, size_t K2>
auto& operator*=(FixedBase<V1, K> &a, const FixedBase<V2, K2>& b) {
    return a = a * b;
}

template<typename V1, size_t K, typename V2, size_t K2>
auto& operator/=(FixedBase<V1, K> &a, const FixedBase<V2, K2>& b) {
    return a = a / b;
}

template<typename V1, size_t K>
auto operator-(FixedBase<V1, K> x) {
    return FixedBase<V1, K>::from_raw(-x.v);
}

template<typename V1, size_t K>
auto abs(FixedBase<V1, K> x) {
    if (x.v < 0) {
        x.v = -x.v;
    }
    return x;
}

template<typename V1, size_t K>
std::ostream &operator<<(std::ostream &out, const FixedBase<V1, K>& x) {
    return out << x.v / (double) (1ll << K);
}
