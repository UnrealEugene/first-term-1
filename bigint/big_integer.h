#pragma once

#include <string>
#include <vector>

/*
 * data__ содержит цифры числа в системе счисления 2^64,
 * записанные начиная с младших цифр. Лидирующие нули
 * отсутствуют, кроме единственного, если число = 0
 *
 * sign__ содержит знак числа (true - отрицательное)
 */

class big_integer {
private:
    using storage_t = std::vector<uint64_t>;

    storage_t data__;
    bool sign__;
    void set_sign(bool);
    void switch_sign();
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
    big_integer& operator/=(big_integer);
    big_integer& operator%=(const big_integer&);

    big_integer& operator>>=(int);
    big_integer& operator<<=(int);
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

    friend big_integer operator<<(big_integer, int);
    friend big_integer operator>>(big_integer, int);
    friend big_integer operator&(const big_integer&, const big_integer&);
    friend big_integer operator|(const big_integer&, const big_integer&);
    friend big_integer operator^(const big_integer&, const big_integer&);

    friend big_integer operator-(const big_integer&);
    friend big_integer operator+(const big_integer&);
    friend big_integer operator~(const big_integer&);

    friend std::ostream& operator<<(std::ostream&, const big_integer&);
    friend std::istream& operator>>(std::istream&, big_integer&);

    friend std::string to_string(big_integer);
};