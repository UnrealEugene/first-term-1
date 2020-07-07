#pragma once

#include <string>
#include <vector>
#include <functional>

/*
 * data_ содержит цифры числа в системе счисления 2^64,
 * записанные начиная с младших цифр. Лидирующие нули
 * отсутствуют, кроме единственного, если число = 0
 *
 * sign_ содержит знак числа (true - отрицательное)
 */

class big_integer {
private:
    using storage_t = std::vector<uint64_t>;
    storage_t data_;
    bool sign_;
    void set_sign_(bool);
    void switch_sign_();
    big_integer abs_() const;
    uint64_t div_short_(uint64_t);
    void two_complement_();
    big_integer& apply_bitwise_(const std::function<uint64_t(uint64_t, uint64_t)>&, big_integer);
    void keep_invariant_();
public:
    big_integer();
    big_integer(const int&);
    big_integer(const long&);
    big_integer(const long long&);
    big_integer(const unsigned&);
    big_integer(const unsigned long&);
    big_integer(const unsigned long long&);
    explicit big_integer(const std::string&);
    big_integer(const big_integer&) = default;

    bool sign() const;

    big_integer& operator=(const big_integer&);

    big_integer& operator+=(const big_integer&);
    big_integer& operator-=(const big_integer&);
    big_integer& operator*=(const big_integer&);
    big_integer& operator/=(const big_integer&);
    big_integer& operator%=(const big_integer&);

    big_integer& operator>>=(uint64_t);
    big_integer& operator<<=(uint64_t);
    big_integer& operator&=(const big_integer&);
    big_integer& operator|=(const big_integer&);
    big_integer& operator^=(const big_integer&);

    big_integer& operator--();
    big_integer operator--(int);
    big_integer& operator++();
    big_integer operator++(int);

    friend bool operator==(const big_integer&, const big_integer&);
    friend bool operator!=(const big_integer&, const big_integer&);
    friend bool operator<(const big_integer&, const big_integer&);
    friend bool operator<=(const big_integer&, const big_integer&);
    friend bool operator>(const big_integer&, const big_integer&);
    friend bool operator>=(const big_integer&, const big_integer&);

    friend big_integer operator+(big_integer, const big_integer&);
    friend big_integer operator-(big_integer, const big_integer&);
    friend big_integer operator*(big_integer, const big_integer&);
    friend big_integer operator/(big_integer, const big_integer&);
    friend big_integer operator%(big_integer, const big_integer&);

    friend big_integer operator<<(big_integer, uint64_t);
    friend big_integer operator>>(big_integer, uint64_t);
    friend big_integer operator&(big_integer, const big_integer&);
    friend big_integer operator|(big_integer, const big_integer&);
    friend big_integer operator^(big_integer, const big_integer&);

    friend big_integer operator-(const big_integer&);
    friend big_integer operator+(const big_integer&);
    friend big_integer operator~(const big_integer&);

    friend std::ostream& operator<<(std::ostream&, const big_integer&);
    friend std::istream& operator>>(std::istream&, big_integer&);

    friend std::string to_string(big_integer);
};