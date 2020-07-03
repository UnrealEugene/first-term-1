#include <string>
#include <limits>
#include <cassert>
#include <algorithm>
#include "big_integer.h"

const uint64_t MAX_DIGIT = std::numeric_limits<uint64_t>::max();
const uint64_t BASE_POWER2 = 64;


static bool add_overflow__(uint64_t left, uint64_t right, bool carry = false) {
    return left > MAX_DIGIT - right || (carry && left + right == MAX_DIGIT);
}

static bool sub_overflow__(uint64_t left, uint64_t right, bool carry = false) {
    return left < right || (carry && left == right);
}

static uint64_t mul_overflow__(uint64_t left, uint64_t right) {
    uint64_t upper, lower;
    __asm__("mulq %3;"
    : "=a" (lower), "=d" (upper)
    : "a" (left), "r" (right));
    return upper;
}

static uint64_t abs_(const int& x) {
    return x >= 0 ? (unsigned) x : -((unsigned) x);
}

static uint64_t labs_(const long& x) {
    return x >= 0 ? (unsigned long) x : -((unsigned long) x);
}

static uint64_t llabs_(const long long& x) {
    return x >= 0 ? (unsigned long long) x : -((unsigned long long) x);
}

big_integer::big_integer() :
        data__({0ULL}), sign__(false) { }

big_integer::big_integer(const int& val) :
        data__({abs_(val)}), sign__(val < 0) { }

big_integer::big_integer(const long& val) :
        data__({labs_(val)}), sign__(val < 0) { }

big_integer::big_integer(const long long& val) :
        data__({llabs_(val)}), sign__(val < 0) { }

big_integer::big_integer(const unsigned& val) :
        data__({val}), sign__(false) { }

big_integer::big_integer(const unsigned long& val) :
        data__({val}), sign__(false) { }

big_integer::big_integer(const unsigned long long& val) :
        data__({val}), sign__(false) { }


big_integer::big_integer(const std::string& str) {
    sign__ = false;
    size_t i = 0;
    bool new_sign;
    if (str[i] == '-') {
        new_sign = true;
        i++;
    } else if (str[i] == '+') {
        new_sign = false;
        i++;
    } else {
        new_sign = false;
    }

    const int DIGITS_COUNT = 19;  // log_10(2^64)
    const uint64_t POWER_10_DIGITS = 10000000000000000000ULL;  // 10^19
    uint64_t buf = 0ULL;
    size_t cur_cnt = 0;
    while (i != str.size()) {
        while (i != str.size() && cur_cnt < DIGITS_COUNT) {
            buf = buf * 10ULL + (uint64_t) (str[i] - '0');
            i++;
            cur_cnt++;
        }
        if (cur_cnt == DIGITS_COUNT) {
            cur_cnt = 0;
            (*this) *= POWER_10_DIGITS;
        } else {
            uint64_t current_power_10 = 1LL;
            while (cur_cnt > 0) {
                cur_cnt--;
                current_power_10 *= 10;
            }
            (*this) *= current_power_10;
        }
        (*this) += buf;
        buf = 0ULL;
    }
    set_sign(new_sign);
}


bool big_integer::sign() const {
    return sign__;
}


void big_integer::set_sign(bool new_sign) {
    if ((*this) != 0)
        sign__ = new_sign;
}


void big_integer::switch_sign() {
    set_sign(!sign__);
}


big_integer& big_integer::operator=(const big_integer& right) {
    if (this == &right)
        return *this;
    data__ = right.data__;
    sign__ = right.sign__;
    return (*this);
}


big_integer& big_integer::operator+=(const big_integer& right) {
    if (!sign() && right.sign()) {
        return (*this) -= -right;
    }
    if (sign() && !right.sign()) {
        switch_sign();
        (*this) -= right;
        switch_sign();
        return (*this);
    }
    data__.resize(std::max(data__.size(), right.data__.size()) + 1);
    bool carry = false;
    for (size_t i = 0; i < data__.size(); i++) {
        uint64_t left_ = (i < data__.size() ? data__[i] : 0ULL);
        uint64_t right_ = (i < right.data__.size() ? right.data__[i] : 0ULL);
        data__[i] = left_ + right_ + (uint64_t) carry;
        carry = add_overflow__(left_, right_, carry);
    }
    while (data__.size() > 1 && data__.back() == 0ULL)
        data__.pop_back();
    return (*this);
}


big_integer& big_integer::operator-=(const big_integer& right) {
    if (!sign() && right.sign()) {
        return (*this) += -right;
    }
    if (sign() && !right.sign()) {
        switch_sign();
        (*this) += right;
        switch_sign();
        return (*this);
    }
    bool new_sign = (*this) < right;
    data__.resize(std::max(data__.size(), right.data__.size()));

    bool carry = false;
    for (size_t i = 0; i < data__.size(); i++) {
        uint64_t left_ = (i < data__.size() ? data__[i] : 0ULL);
        uint64_t right_ = (i < right.data__.size() ? right.data__[i] : 0ULL);
        if (sign() ^ new_sign)
            std::swap(left_, right_);
        data__[i] = left_ - right_ - (uint64_t) carry;
        carry = sub_overflow__(left_, right_, carry);
    }
    set_sign(new_sign);
    while (data__.size() > 1 && data__.back() == 0ULL)
        data__.pop_back();
    return (*this);
}


big_integer& big_integer::operator*=(const big_integer& right) {
    big_integer result;
    result.data__.resize(data__.size() + right.data__.size());

    for (size_t i = 0; i < data__.size(); i++) {
        uint64_t carry = 0;
        for (size_t j = 0; j < right.data__.size() || carry > 0; j++) {
            uint64_t left_ = data__[i];
            uint64_t right_ = j < right.data__.size() ? right.data__[j] : 0;
            uint64_t upper = mul_overflow__(left_, right_);
            uint64_t lower = left_ * right_;
            upper += add_overflow__(lower, carry);
            lower += carry;
            upper += add_overflow__(lower, result.data__[i + j]);
            lower += result.data__[i + j];
            carry = upper;
            result.data__[i + j] = lower;
        }
    }

    result.set_sign(sign() ^ right.sign());

    while (result.data__.size() > 1 && result.data__.back() == 0ULL)
        result.data__.pop_back();
    return (*this) = result;
}


big_integer& big_integer::operator/=(big_integer right) {
    assert(right != 0);
    big_integer result, remainder;
    bool new_sign = sign() ^ right.sign();
    right.set_sign(false);
    for (size_t i = data__.size(); i --> 0; ) {
        remainder = (remainder << BASE_POWER2) + data__[i];
        uint64_t l = 0, r = MAX_DIGIT;
        while (l < r) {
            uint64_t m = r - (r - l) / 2;
            if (remainder < right * m)
                r = m - 1;
            else
                l = m;
        }
        result = (result << BASE_POWER2) + l;
        remainder -= right * l;
    }
    result.set_sign(new_sign);
    return (*this) = result;
}


big_integer& big_integer::operator%=(const big_integer& right) {
    bool new_sign = sign();
    (*this) -= (*this) / right * right;
    set_sign(new_sign);
    return (*this);
}


// 110101 -> 1101
//

big_integer& big_integer::operator>>=(int right) {
    assert(right >= 0);
    if (sign())
        ++(*this);
    size_t right_bits = right / BASE_POWER2;
    if (right_bits < data__.size()) {
        std::reverse(data__.begin(), data__.end());
        while (right_bits--)
            data__.pop_back();
        std::reverse(data__.begin(), data__.end());
    } else {
        data__ = {0ULL};
    }
    right %= BASE_POWER2;
    (*this) /= 1ULL << (unsigned) right;
    if (sign())
        --(*this);
    return (*this);
}


big_integer& big_integer::operator<<=(int right) {
    assert(right >= 0);
    if ((*this) == 0)
        return (*this);
    const size_t right_bits = right / BASE_POWER2;
    if (right_bits > 0) {
        data__.resize(data__.size() + right_bits);
        std::move_backward(data__.begin(), data__.end() - right_bits, data__.end());
        std::fill(data__.begin(), data__.begin() + right_bits, 0ULL);
        right %= BASE_POWER2;
    }
    (*this) *= 1ULL << (unsigned) right;
    return (*this);
}


big_integer& big_integer::operator&=(const big_integer& right) {
    return (*this) = (*this) & right;
}


big_integer& big_integer::operator|=(const big_integer& right) {
    return (*this) = (*this) | right;
}


big_integer& big_integer::operator^=(const big_integer& right) {
    return (*this) = (*this) ^ right;
}


big_integer& big_integer::operator--() {
    return (*this) -= 1;
}

big_integer big_integer::operator--(int) {
    big_integer t(*this);
    (*this) -= 1;
    return t;
}


big_integer& big_integer::operator++() {
    return (*this) += 1;
}

big_integer big_integer::operator++(int) {
    big_integer t(*this);
    (*this) += 1;
    return t;
}


bool operator==(const big_integer& left, const big_integer& right) {
    return !(left != right);
}


bool operator!=(const big_integer& left, const big_integer& right) {
    return left < right || right < left;
}


bool operator<(const big_integer& left, const big_integer& right) {
    if (left.sign() && !right.sign())
        return true;
    if (!left.sign() && right.sign())
        return false;
    if (left.data__.size() != right.data__.size())
        return left.sign() ? left.data__.size() > right.data__.size() : left.data__.size() < right.data__.size();
    for (size_t i = left.data__.size(); i --> 0; ) {
        if (left.data__[i] != right.data__[i]) {
            return left.sign() ? left.data__[i] > right.data__[i] : left.data__[i] < right.data__[i];
        }
    }
    return false;
}


bool operator<=(const big_integer& left, const big_integer& right) {
    return !(left > right);
}


bool operator>(const big_integer& left, const big_integer& right) {
    return right < left;
}


bool operator>=(const big_integer& left, const big_integer& right) {
    return !(left < right);
}


big_integer operator+(big_integer left, const big_integer& right) {
    return left += right;
}


big_integer operator-(big_integer left, const big_integer& right) {
    return left -= right;
}


big_integer operator*(big_integer left, const big_integer& right) {
    return left *= right;
}


big_integer operator/(big_integer left, const big_integer& right) {
    return left /= right;
}


big_integer operator%(big_integer left, const big_integer& right) {
    return left %= right;
}


big_integer operator<<(big_integer left, int right) {
    return left <<= right;
}


big_integer operator>>(big_integer left, int right) {
    return left >>= right;
}


big_integer operator&(const big_integer& left, const big_integer& right) {
    if (!left.sign() && right.sign())
        return right & left;
    if (left.sign() && right.sign())
        return ~(~left | ~right);
    big_integer _left = left, _right = right;
    if (_left.sign() && !_right.sign()) {
        ++_left;
        const size_t CNT = std::min(_left.data__.size(), _right.data__.size());
        for (size_t i = 0; i < CNT; i++)
            _right.data__[i] &= ~_left.data__[i];
        return _right;
    }
    // if (!_left.sign && !_right.sign)
    if (_left.data__.size() > _right.data__.size())
        std::swap(_left, _right);
    for (size_t i = 0; i < _left.data__.size(); i++)
        _left.data__[i] &= _right.data__[i];
    return _left;
}


big_integer operator|(const big_integer& left, const big_integer& right) {
    if (left.sign() || right.sign())
        return ~(~left & ~right);
    // if (!_left.sign && !_right.sign)
    big_integer _left = left, _right = right;
    if (_left.data__.size() < _right.data__.size())
        std::swap(_left, _right);
    for (size_t i = 0; i < _right.data__.size(); i++)
        _left.data__[i] |= _right.data__[i];
    return _left;
}


big_integer operator^(const big_integer& left, const big_integer& right) {
    return (~left & right) | (left & ~right);
}


big_integer operator-(const big_integer& val) {
    big_integer result(val);
    result.switch_sign();
    return result;
}


big_integer operator+(const big_integer& val) {
    return big_integer(val);
}


big_integer operator~(const big_integer& val) {
    return -val - 1;
}


std::string to_string(big_integer arg) {
    std::string res;
    bool neg = arg.sign();
    arg.set_sign(false);
    do {
        big_integer new_arg = arg;
        new_arg /= 10;
        res.push_back(static_cast<char>((arg - new_arg * 10).data__[0] + '0'));
        std::swap(arg, new_arg);
    } while (arg != 0);
    if (neg)
        res.push_back('-');
    std::reverse(res.begin(), res.end());
    return res;
}


std::istream& operator>>(std::istream& in, big_integer& num) {
    std::string str;
    in >> str;
    num = big_integer(str);
    return in;
}


std::ostream& operator<<(std::ostream& out, const big_integer& num) {
    return out << to_string(num);
}