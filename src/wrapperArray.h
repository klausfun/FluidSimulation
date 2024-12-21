#pragma once

#include <vector>
#include <cstring>
#include <iostream>

template <typename T, int NVal, int KVal>
struct Array {
    T v[NVal][KVal]{};

    void init(int N, int K);
    T* operator[](int index);
    Array& operator=(const Array& b);
};

template <typename T>
struct Array<T, 0, 0> {
    std::vector<std::vector<T>> v;

    void init(int N, int K);
    std::vector<T>& operator[](int index);
    Array& operator=(const Array& b);
};



template <typename T, int NVal, int KVal>
void Array<T, NVal, KVal>::init(int N, int K) {
    if (N != NVal || K != KVal) {std::cout << "Wrong size of field\n"; throw std::exception();}
}

template <typename T>
void Array<T, 0, 0>::init(int N, int K) {
    v.resize(N, std::vector<T>(K));
}

template <typename T, int NVal, int KVal>
T* Array<T, NVal, KVal>::operator[](int index) {
    return v[index];
}

template <typename T>
std::vector<T>& Array<T, 0, 0>::operator[](int index) {
    return v[index];
}


template <typename T, int NVal, int KVal>
Array<T, NVal, KVal>& Array<T, NVal, KVal>::operator=(const Array& other) {
    if(this == &other) {return *this;}
    memcpy(v, other.v, sizeof(v));
    return *this;
}

template <typename T>
Array<T, 0, 0>& Array<T, 0, 0>::operator=(const Array& other) {
    v = other.v;
    return *this;
}