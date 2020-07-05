#include <string>
#include <limits>
#include <cassert>
#include <algorithm>
#include <tuple>
#include "big_integer.h"

const uint64_t MAX_DIGIT = std::numeric_limits<uint64_t>::max();
const uint64_t BASE_POWER2 = 64;

const big_integer ZERO;

static bool add_overflow_(uint64_t left, uint64_t right, bool carry = false) {
    return left > MAX_DIGIT - right || (carry && left + right == MAX_DIGIT);
}

static bool sub_overflow_(uint64_t left, uint64_t right, bool carry = false) {
    return left < right || (carry && left == right);
}

static uint64_t mul_overflow_(uint64_t left, uint64_t right) {
    uint64_t upper, lower;
    __asm__("mulq %3;"
    : "=a" (lower), "=d" (upper)
    : "a" (left), "r" (right));
    return upper;
}

// if upper_left >= right || right == 0 then UB
static std::pair<uint64_t, uint64_t> div_mod_(uint64_t upper_left, uint64_t lower_left, uint64_t right) {
    uint64_t result, modulo;
    __asm__("divq %4;"
    : "=a" (result), "=d" (modulo)
    : "a" (lower_left), "d" (upper_left), "r" (right));
    return {result, modulo};
}

static uint64_t soft_div(uint64_t upper_left, uint64_t lower_left, uint64_t right) {
    assert(right != 0);
    if (upper_left >= right) {
        return MAX_DIGIT;
    }
    return div_mod_(upper_left, lower_left, right).first;
}

static uint64_t iabs_(const int& x) {
    return x >= 0 ? static_cast<unsigned>(x) : -static_cast<unsigned>(x);
}

static uint64_t labs_(const long& x) {
    return x >= 0 ? static_cast<unsigned long>(x) : -static_cast<unsigned long>(x);
}

static uint64_t llabs_(const long long& x) {
    return x >= 0 ? static_cast<unsigned long long>(x) : -static_cast<unsigned long long>(x);
}

big_integer::big_integer() :
        data_({0ULL}), sign_(false) { }

big_integer::big_integer(const int& val) :
        data_({iabs_(val)}), sign_(val < 0) { }

big_integer::big_integer(const long& val) :
        data_({labs_(val)}), sign_(val < 0) { }

big_integer::big_integer(const long long& val) :
        data_({llabs_(val)}), sign_(val < 0) { }

big_integer::big_integer(const unsigned& val) :
        data_({val}), sign_(false) { }

big_integer::big_integer(const unsigned long& val) :
        data_({val}), sign_(false) { }

big_integer::big_integer(const unsigned long long& val) :
        data_({val}), sign_(false) { }


big_integer::big_integer(const std::string& str) {
    sign_ = false;
    size_t i = str[0] == '-' || str[0] == '+';
    bool new_sign = str[0] == '-';

    const int DIGITS_COUNT = 19;  // 19 -- max power of 10 less than 2^64
    const uint64_t POWER_10_DIGITS = 10000000000000000000ULL;  // 10^19 -- max power of 10 less than 2^64
    uint64_t buf = 0ULL;
    size_t cur_cnt = 0;
    while (i != str.size()) {
        while (i != str.size() && cur_cnt < DIGITS_COUNT) {
            buf = buf * 10ULL + static_cast<uint64_t>(str[i] - '0');
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
    set_sign_(new_sign);
}


bool big_integer::sign() const {
    return sign_;
}


void big_integer::set_sign_(bool new_sign) {
    sign_ = (data_.size() != 1 || data_[0] != 0) && new_sign;
}


void big_integer::switch_sign_() {
    set_sign_(!sign_);
}


big_integer big_integer::abs_() const {
    return sign() ? -(*this) : (*this);
}


uint64_t big_integer::div_short_(uint64_t right) {
    assert(right != 0);
    uint64_t carry = 0;
    for (size_t i = data_.size(); i --> 0; ) {
        std::tie(data_[i], carry) = div_mod_(carry, data_[i], right);
    }
    keep_invariant_();
    return carry;
}


void big_integer::two_complement_() {
    if (sign()) {
        ++(*this);
        for (uint64_t& i : data_) {
            i = ~i;
        }
    }
}


big_integer& big_integer::apply_bitwise_(const std::function<uint64_t(uint64_t, uint64_t)>& f, big_integer right) {
    size_t sz = std::max(data_.size(), right.data_.size());
    bool new_sign = static_cast<bool>(f(sign(), right.sign()) & 1ULL);

    right.two_complement_();
    two_complement_();
    for (size_t i = 0; i < sz; i++) {
        uint64_t l = i < data_.size() ? data_[i] : sign() ? MAX_DIGIT : 0ULL;
        uint64_t r = i < right.data_.size() ? right.data_[i] : right.sign() ? MAX_DIGIT : 0ULL;
        if (i == data_.size()) {
            data_.push_back(0ULL);
        }
        data_[i] = f(l, r);
    }
    set_sign_(new_sign);
    two_complement_();
    keep_invariant_();
    return (*this);
}


void big_integer::keep_invariant_() {
    while (data_.size() > 1 && data_.back() == 0) {
        data_.pop_back();
    }
    set_sign_(sign_);
}


big_integer& big_integer::operator=(const big_integer& right) {
    if (this == &right) {
        return *this;
    }
    data_ = right.data_;
    sign_ = right.sign_;
    return (*this);
}


big_integer& big_integer::operator+=(const big_integer& right) {
    if (!sign() && right.sign()) {
        return (*this) -= -right;
    }
    if (sign() && !right.sign()) {
        switch_sign_();
        (*this) -= right;
        switch_sign_();
        return (*this);
    }
    data_.resize(std::max(data_.size(), right.data_.size()) + 1);
    uint64_t carry = false;
    for (size_t i = 0; i < data_.size(); i++) {
        uint64_t left_ = (i < data_.size() ? data_[i] : 0ULL);
        uint64_t right_ = (i < right.data_.size() ? right.data_[i] : 0ULL);
        data_[i] = left_ + right_ + carry;
        carry = add_overflow_(left_, right_, carry);
    }
    keep_invariant_();
    return (*this);
}


big_integer& big_integer::operator-=(const big_integer& right) {
    if (!sign() && right.sign()) {
        return (*this) += -right;
    }
    if (sign() && !right.sign()) {
        switch_sign_();
        (*this) += right;
        switch_sign_();
        return (*this);
    }
    bool new_sign = (*this) < right;
    data_.resize(std::max(data_.size(), right.data_.size()));

    uint64_t carry = false;
    for (size_t i = 0; i < data_.size(); i++) {
        uint64_t left_ = (i < data_.size() ? data_[i] : 0ULL);
        uint64_t right_ = (i < right.data_.size() ? right.data_[i] : 0ULL);
        if (sign() ^ new_sign) {
            std::swap(left_, right_);
        }
        data_[i] = left_ - right_ - carry;
        carry = sub_overflow_(left_, right_, carry);
    }
    set_sign_(new_sign);
    keep_invariant_();
    return (*this);
}


big_integer& big_integer::operator*=(const big_integer& right) {
    big_integer result;
    result.data_.resize(data_.size() + right.data_.size());

    for (size_t i = 0; i < data_.size(); i++) {
        uint64_t carry = 0;
        for (size_t j = 0; j < right.data_.size() || carry > 0; j++) {
            uint64_t left_ = data_[i];
            uint64_t right_ = j < right.data_.size() ? right.data_[j] : 0;
            uint64_t upper = mul_overflow_(left_, right_);
            uint64_t lower = left_ * right_;
            upper += add_overflow_(lower, carry);
            lower += carry;
            upper += add_overflow_(lower, result.data_[i + j]);
            lower += result.data_[i + j];
            carry = upper;
            result.data_[i + j] = lower;
        }
    }
    result.set_sign_(sign() ^ right.sign());
    result.keep_invariant_();
    return (*this) = result;
}


big_integer& big_integer::operator/=(const big_integer& right) {
    assert(right != ZERO);

    if (this->abs_() < right.abs_()) {
        return (*this) = 0;
    }

    bool new_sign = sign() ^ right.sign();
    if (right.data_.size() == 1) {
        div_short_(right.data_[0]);
        set_sign_(new_sign);
        return (*this);
    }

    const uint64_t NORM_FACTOR = right.data_.back() == MAX_DIGIT ? 1 : MAX_DIGIT / (right.data_.back() + 1);
    big_integer u = (*this) * NORM_FACTOR;
    big_integer d = right * NORM_FACTOR;

    u.data_.push_back(0);
    u.set_sign_(false);
    d.set_sign_(false);

    size_t n = u.data_.size(), m = d.data_.size();
    data_.resize(n - m);
    set_sign_(new_sign);

    big_integer dq;
    for (size_t k = n - m; k --> 0; ) {
        uint64_t qt = soft_div(k + m < u.data_.size() ? u.data_[k + m] : 0,
                               k + m - 1 < u.data_.size() ? u.data_[k + m - 1] : 0,
                               d.data_[m - 1]);
        dq = d * qt;
        while (qt != 0 && u < dq << BASE_POWER2 * k) {
            --qt;
            dq -= d;
        }
        data_[k] = qt;
        u -= dq << BASE_POWER2 * k;
    }
    keep_invariant_();
    return (*this);
}


big_integer& big_integer::operator%=(const big_integer& right) {
    (*this) -= (*this) / right * right;
    return (*this);
}


big_integer& big_integer::operator>>=(uint64_t right) {
    if (sign()) {
        ++(*this);
    }
    size_t right_bits = right / BASE_POWER2;
    if (right_bits < data_.size()) {
        std::reverse(data_.begin(), data_.end());
        while (right_bits--) {
            data_.pop_back();
        }
        std::reverse(data_.begin(), data_.end());
    } else {
        data_ = {0ULL};
    }
    right %= BASE_POWER2;
    (*this) /= 1ULL << right;
    if (sign())
        --(*this);
    return (*this);
}


big_integer& big_integer::operator<<=(uint64_t right) {
    if ((*this) == 0) {
        return (*this);
    }
    const size_t RIGHT_BITS = right / BASE_POWER2;
    data_.resize(data_.size() + RIGHT_BITS);
    std::move_backward(data_.begin(), data_.end() - RIGHT_BITS, data_.end());
    std::fill(data_.begin(), data_.begin() + RIGHT_BITS, 0ULL);
    right %= BASE_POWER2;
    return (*this) *= 1ULL << right;
}


big_integer& big_integer::operator&=(const big_integer& right) {
    return apply_bitwise_([](uint64_t a, uint64_t b) { return a & b; }, right);
}


big_integer& big_integer::operator|=(const big_integer& right) {
    return apply_bitwise_([](uint64_t a, uint64_t b) { return a | b; }, right);
}


big_integer& big_integer::operator^=(const big_integer& right) {
    return apply_bitwise_([](uint64_t a, uint64_t b) { return a ^ b; }, right);
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
    if (left.sign() && !right.sign()) {
        return true;
    }
    if (!left.sign() && right.sign()) {
        return false;
    }
    for (size_t i = std::max(left.data_.size(), right.data_.size()); i --> 0; ) {
        uint64_t l = i < left.data_.size() ? left.data_[i] : 0ULL;
        uint64_t r = i < right.data_.size() ? right.data_[i] : 0ULL;
        if (l != r) {
            return left.sign() ? l > r : l < r;
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


big_integer operator<<(big_integer left, uint64_t right) {
    return left <<= right;
}


big_integer operator>>(big_integer left, uint64_t right) {
    return left >>= right;
}


big_integer operator&(big_integer left, const big_integer& right) {
    return left &= right;
}


big_integer operator|(big_integer left, const big_integer& right) {
    return left |= right;
}


big_integer operator^(big_integer left, const big_integer& right) {
    return left ^= right;
}


big_integer operator-(const big_integer& val) {
    big_integer result(val);
    result.switch_sign_();
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
    do {
        res.push_back(static_cast<char>(arg.div_short_(10) + '0'));
    } while (arg != ZERO);
    if (neg) {
        res.push_back('-');
    }
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