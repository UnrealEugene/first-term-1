//
// Created by eugene on 07.07.2020.
//
#pragma once

#include <vector>

template <typename T>
struct vector_ptr {
    vector_ptr()
            : ptr_(new ref_data_()) { }

    vector_ptr(const std::vector<T>& x = T())
            : ptr_(new ref_data_(x)) { }

    vector_ptr(const T* first, const T* last)
            : ptr_(new ref_data_(first, last)) { }

    vector_ptr(const vector_ptr<T>& other) {
        share(other);
    }

    vector_ptr& operator=(const vector_ptr<T>& other) {
        if (this != &other) {
            unshare();
            share(other);
        }
        return (*this);
    }

    ~vector_ptr() {
        unshare();
    }

    std::vector<T>* operator->() {
        return &ptr_->obj_;
    }

    const std::vector<T>* operator->() const {
        return &ptr_->obj_;
    }

    std::vector<T>& operator*() {
        return ptr_->obj_;
    }

    const std::vector<T>& operator*() const {
        return ptr_->obj_;
    }

    void detach() {
        if (ptr_->ref_cnt_ > 1) {
            --ptr_->ref_cnt_;
            ptr_ = new ref_data_(ptr_->obj_);
        }
    }

 private:
    void unshare() {
        --ptr_->ref_cnt_;
        if (ptr_->ref_cnt_ == 0) {
            delete ptr_;
        }
    }

    void share(const vector_ptr<T>& other) {
        ptr_ = other.ptr_;
        ++ptr_->ref_cnt_;
    }

    struct ref_data_ {
        std::vector<T> obj_;
        size_t ref_cnt_;

        ref_data_()
                : obj_(), ref_cnt_(1) { }

        explicit ref_data_(const std::vector<T>& other)
                : obj_(other), ref_cnt_(1) { }

        ref_data_(const T* first, const T* last)
                : obj_(first, last), ref_cnt_(1) { }
    } *ptr_;
};