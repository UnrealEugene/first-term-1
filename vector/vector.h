//
// Created by eugene on 16.05.2020.
//
#pragma once
#include <cstddef>
#include <cstring>
#include <utility>
#include <cassert>

template <typename T>
struct vector {
    typedef T* iterator;
    typedef T const* const_iterator;

    vector();                               // O(1) nothrow
    vector(const vector&);                  // O(N) strong
    vector& operator=(const vector& other); // O(N) strong

    ~vector();                              // O(N) nothrow

    T& operator[](size_t i);                // O(1) nothrow
    T const& operator[](size_t i) const;    // O(1) nothrow

    T* data();                              // O(1) nothrow
    T const* data() const;                  // O(1) nothrow
    size_t size() const;                    // O(1) nothrow

    T& front();                             // O(1) nothrow
    T const& front() const;                 // O(1) nothrow

    T& back();                              // O(1) nothrow
    T const& back() const;                  // O(1) nothrow
    void push_back(T const&);               // O(1)* strong
    void pop_back();                        // O(1) nothrow

    bool empty() const;                     // O(1) nothrow

    size_t capacity() const;                // O(1) nothrow
    void reserve(size_t);                   // O(N) strong
    void shrink_to_fit();                   // O(N) strong

    void clear();                           // O(N) nothrow

    void swap(vector&);                     // O(1) nothrow

    iterator begin();                       // O(1) nothrow
    iterator end();                         // O(1) nothrow

    const_iterator begin() const;           // O(1) nothrow
    const_iterator end() const;             // O(1) nothrow

    iterator insert(const_iterator pos, const T&); // O(N) weak
    iterator erase(const_iterator pos);     // O(N) weak
    iterator erase(const_iterator first, const_iterator last); // O(N) weak

private:
    void force_capacity(size_t);            // O(N) strong
    static T* copy_data(const T*, size_t, size_t); // O(N) strong
    static void free_data(T*, size_t); // O(N) nothrow

    T* data_;
    size_t size_;
    size_t capacity_;
};

template <typename T>
vector<T>::vector() : data_(nullptr), size_(0), capacity_(0) { }

template <typename T>
vector<T>::vector(const vector& other) {
    data_ = copy_data(other.data_, other.size_, other.size_);
    size_ = other.size_;
    capacity_ = other.size_;
}

template <typename T>
vector<T>& vector<T>::operator=(const vector& other) {
    if (this == &other)
        return (*this);
    vector t = other;
    swap(t);
    return (*this);
}

template <typename T>
vector<T>::~vector() {
    clear();
    operator delete(data_);
}

template <typename T>
T& vector<T>::operator[](size_t i) {
    return data_[i];
}

template <typename T>
T const& vector<T>::operator[](size_t i) const {
    return data_[i];
}

template <typename T>
void vector<T>::swap(vector& other) {
    std::swap(data_, other.data_);
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);
}

template <typename T>
T* vector<T>::data() {
    return data_;
}

template <typename T>
T const* vector<T>::data() const {
    return data_;
}

template <typename T>
size_t vector<T>::size() const {
    return size_;
}

template <typename T>
T& vector<T>::front() {
    return data_[0];
}

template<typename T>
T const& vector<T>::front() const {
    return data_[0];
}

template <typename T>
T& vector<T>::back() {
    return data_[size_ - 1];
}

template <typename T>
T const& vector<T>::back() const {
    return data_[size_ - 1];
}

template<typename T>
void vector<T>::push_back(const T& elem) {
    if (size_ == capacity_) {
        T t = elem;
        force_capacity(capacity_ ? 2 * capacity_ : 1);
        new (data_ + size_) T(t);
    } else {
        new (data_ + size_) T(elem);
    }
    size_++;
}

template<typename T>
void vector<T>::pop_back() {
    data_[--size_].~T();
}

template <typename T>
bool vector<T>::empty() const {
    return size_ == 0;
}

template <typename T>
size_t vector<T>::capacity() const {
    return capacity_;
}

template <typename T>
void vector<T>::reserve(size_t new_capacity) {
    if (new_capacity <= capacity_)
        return;
    force_capacity(new_capacity);
}

template<typename T>
void vector<T>::shrink_to_fit() {
    if (empty()) {
        operator delete(data_);
        data_ = nullptr;
    } else {
        force_capacity(size_);
    }
}

template<typename T>
void vector<T>::clear() {
    free_data(data_, size_);
    size_ = 0;
}

template<typename T>
typename vector<T>::iterator vector<T>::begin() {
    return data_;
}

template<typename T>
typename vector<T>::iterator vector<T>::end() {
    return data_ + size_;
}

template<typename T>
typename vector<T>::const_iterator vector<T>::begin() const {
    return data_;
}

template <typename T>
typename vector<T>::const_iterator vector<T>::end() const {
    return data_ + size_;
}

template <typename T>
typename vector<T>::iterator vector<T>::insert(vector::const_iterator pos, const T& elem) {
    size_t ind = pos - begin();
    push_back(elem);
    pos = begin() + ind;
    for (const_iterator it = end() - 1; it --> pos; )
        std::swap(*const_cast<iterator>(it), *const_cast<iterator>(it + 1));
    return const_cast<iterator>(pos);
}

template <typename T>
void vector<T>::force_capacity(size_t new_cap) {
    assert(new_cap >= size_);
    if (new_cap == capacity_) {
        return;
    }
    T* tmp = copy_data(data_, size_, new_cap);
    free_data(data_, size_);
    operator delete(data_);
    data_ = tmp;
    capacity_ = new_cap;
}

template<typename T>
typename vector<T>::iterator vector<T>::erase(vector::const_iterator pos) {
    return erase(pos, pos + 1);
}

template<typename T>
typename vector<T>::iterator vector<T>::erase(vector::const_iterator first, vector::const_iterator last) {
    if (first < last) {
        size_t d = last - first;
        for (const_iterator it = first; it < end() - d; it++) {
            std::swap(*const_cast<iterator>(it), *const_cast<iterator>(it + d));
        }
        while (d--) {
            pop_back();
        }
    }
    return const_cast<iterator>(first);
}

template<typename T>
T* vector<T>::copy_data(const T* src, size_t new_size, size_t new_cap) {
    assert(new_cap >= new_size);
    if (new_cap == 0) {
        return nullptr;
    }
    T* tmp = static_cast<T*>(operator new(sizeof(T) * new_cap));
    size_t i = 0;
    try {
        for (; i < new_size; i++) {
            new (tmp + i) T(src[i]);
        }
    } catch (...) {
        free_data(tmp, i);
        operator delete(tmp);
        throw;
    }
    return tmp;
}

template<typename T>
void vector<T>::free_data(T* dst, size_t size) {
    for (size_t i = size; i --> 0; ) {
        dst[i].~T();
    }
}

