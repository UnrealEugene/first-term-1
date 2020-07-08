//
// Created by eugene on 16.05.2020.
//
#pragma once

#include <memory>
#include <vector>
#include <iostream>
#include <algorithm>
#include "vector_ptr.h"

template <typename T, typename = void>
struct uint_storage;

template <typename T>
struct uint_storage<T, typename std::enable_if<std::is_unsigned<T>::value>::type> {
    using iterator = T*;
    using const_iterator = const T*;

    uint_storage() {
        is_big_ = false;
        size_ = 0;
    }

    uint_storage(size_t sz, const T& elem) : uint_storage() {
        while (sz--) {
            push_back(elem);
        }
    }

    uint_storage(const uint_storage<T>& other) {
        if (other.is_big_) {
            init_big_data(other);
        } else {
            init_small_data(other);
        }
        is_big_ = other.is_big_;
    }

    uint_storage& operator=(const uint_storage<T>& other) {
        if (this == &other) {
            return (*this);
        }
        if (other.is_big_) {
            if (is_big_) {
                big_data_ = other.big_data_;
            } else {
                init_big_data(other);
            }
        } else {
            if (is_big_) {
                big_data_.~vector_ptr();
            }
            init_small_data(other);
        }
        is_big_ = other.is_big_;
        return (*this);
    }

    ~uint_storage() {
        if (is_big_) {
            big_data_.~vector_ptr();
        }
    }

    size_t size() const {
        return is_big_ ? big_data_->size() : size_;
    }

    void push_back(const T& elem) {
        if (!is_big_ && size_ == SMALL_DATA_SIZE) {
            small_to_big();
        }
        if (is_big_) {
            big_data_.detach();
            big_data_->push_back(elem);
        } else {
            small_data_[size_++] = elem;
        }
    }

    void pop_back() {
        if (is_big_) {
            big_data_.detach();
            big_data_->pop_back();
        } else {
            --size_;
        }
    }

    T& operator[](size_t ind) {
        if (is_big_) {
            big_data_.detach();
            return (*big_data_)[ind];
        }
        return small_data_[ind];
    }

    const T& operator[](size_t ind) const {
        return is_big_ ? (*big_data_)[ind] : small_data_[ind];
    }

    void resize(size_t new_size) {
        if (!is_big_ && new_size > SMALL_DATA_SIZE) {
            small_to_big();
        }
        if (is_big_) {
            big_data_.detach();
            big_data_->resize(new_size);
        } else {
            std::fill(small_data_ + size_, small_data_ + new_size, 0);
            size_ = new_size;
        }
    }

    T& back() {
        if (is_big_) {
            big_data_.detach();
            return big_data_->back();
        }
        return small_data_[size_ - 1];
    }

    const T& back() const {
        return is_big_ ? big_data_->back() : small_data_[size_ - 1];
    }

    iterator begin() {
        if (is_big_) {
            big_data_.detach();
            return big_data_->data();
        }
        return small_data_;
    }

    iterator end() {
        if (is_big_) {
            big_data_.detach();
            return big_data_->data() + big_data_->size();
        }
        return small_data_ + size_;
    }

    const_iterator begin() const {
        return is_big_ ? big_data_->data() : small_data_;
    }

    const_iterator end() const {
        return is_big_ ? big_data_->data() + big_data_->size() : small_data_ + size_;
    }

 private:
    void small_to_big() {
        if (!is_big_) {
            new (&big_data_) vector_ptr<T>(small_data_, small_data_ + size_);
            is_big_ = true;
        }
    }

    void init_big_data(const uint_storage<T>& src) {
        new (&big_data_) vector_ptr<T>(src.big_data_);
    }

    void init_small_data(const uint_storage<T>& src) {
        std::copy_n(src.small_data_, src.size_, small_data_);
        size_ = src.size_;
    }

    bool is_big_;
    size_t size_; // значение size_ актуально только если is_big_ == false

    static constexpr size_t SMALL_DATA_SIZE = sizeof(vector_ptr<T>) / sizeof(T);

    union {
        vector_ptr<T> big_data_;
        T small_data_[SMALL_DATA_SIZE];
    };
};