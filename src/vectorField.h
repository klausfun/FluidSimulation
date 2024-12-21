#pragma once

#include <utility>
#include <ranges>
#include <cassert>

#include "utils.h"
#include "wrapperArray.h"

template<typename T, int NVal, int KVal>
struct VectorField {
    size_t N = NVal, K = KVal;
    Array<std::array<T, deltas.size()>, NVal, KVal> v;

    T &add(int x, int y, int dx, int dy, T dv) {
        return get(x, y, dx, dy) += dv;
    }

    T &get(int x, int y, int dx, int dy) {
//        size_t i = std::ranges::find(deltas, std::pair(dx, dy)) - deltas.begin();
//        assert(i < deltas.size());
//        return v[x][y][i];

        switch (dx*4 + dy) {
            case -1:
                return v[x][y][0];
            case 1:
                return v[x][y][1];
            case -4:
                return v[x][y][2];
            case 4:
                return v[x][y][3];
        }
    }

    void clear();
    void init(size_t n, size_t k);
};

template <typename Type, int NVal, int KVal>
void VectorField<Type, NVal, KVal>::clear() {
    for (size_t x = 0; x < N; x++) {
        for (size_t y = 0; y < K; y++) {
            for (size_t z = 0; z < deltas.size(); z++) {
                v[x][y][z] = Type();
            }
        }
    }
}

template <typename Type, int NVal, int KVal>
void VectorField<Type, NVal, KVal>::init(size_t n, size_t k) {
    N = n; K = k; v.init(n, k);
}