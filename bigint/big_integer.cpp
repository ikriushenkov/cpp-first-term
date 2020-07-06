#include "big_integer.h"

#include <cstring>
#include <stdexcept>
#include <climits>
#include <algorithm>
#include <cmath>

using uint128 = unsigned __int128;

const uint128 BASE = static_cast<uint128>(UINT32_MAX) + 1;

const size_t STEP = 9;
const uint32_t BASE_STRING = 1000000000;
const big_integer ZERO = big_integer(0);

big_integer::big_integer() : sign(false), data_(1, 0) {}

big_integer::big_integer(big_integer const& other) = default;

big_integer::big_integer(int a) : sign(a < 0) {
    if (a == INT_MIN) {
        data_.push_back(static_cast<uint32_t>(INT_MAX) + 1);
    } else {
        data_.push_back(std::abs(a));
    }
}

big_integer::big_integer(uint32_t a) : sign(false), data_(1, a) {}

big_integer::big_integer(std::string const& str) : big_integer() {
    for (size_t i = (str[0] == '-'); i < str.size(); i += STEP) {
        uint32_t t = 0;
        size_t cur = STEP;
        if (str.size() - i < STEP) {
            cur = str.size() - i;
            *this *= static_cast<uint32_t>(std::pow(10, cur));
        } else {
            *this *= BASE_STRING;
        }
        for (size_t j = 0; j < cur; ++j) {
            t = t * 10 + str[i + j] - '0';
        }
        *this += t;
    }
    sign = str[0] == '-';
    remove_zeros();
}

big_integer::~big_integer() = default;

big_integer& big_integer::operator=(big_integer const& other) = default;

bool operator==(big_integer const& a, big_integer const& b) {
    return (a.sign == b.sign) && (a.data_ == b.data_);
}

bool operator!=(big_integer const& a, big_integer const& b) {
    return !(a == b);
}

bool operator<(big_integer const& a, big_integer const& b) {
    if (a.sign ^ b.sign) {
        return a.sign;
    }
    if (a.data_.size() != b.data_.size()) {
        return a.sign ^ (a.data_.size() < b.data_.size());
    }
    for (size_t i = a.data_.size(); i > 0; --i) {
        if (a.data_[i - 1] != b.data_[i - 1]) {
            return a.sign ^ (a.data_[i - 1] < b.data_[i - 1]);
        }
    }
    return false;
}

bool operator>(big_integer const& a, big_integer const& b) {
    return !(a < b || a == b);
}

bool operator<=(big_integer const& a, big_integer const& b) {
    return !(a > b);
}

bool operator>=(big_integer const& a, big_integer const& b) {
    return !(a < b);
}

uint32_t overflow(uint64_t n) {
    return (n >> 32u);
}

big_integer& big_integer::operator+=(big_integer const& rhs) {
    if (sign) {
        *this = rhs.sign ? -(-*this + -rhs) : (rhs - -*this);
        return *this;
    } else {
        if (rhs.sign) {
            *this -= -rhs;
            return *this;
        }
    }
    if (*this < rhs) {
        *this = rhs + *this;
        return *this;
    }
    uint32_t of = 0;
    for (size_t i = 0; i < data_.size(); ++i) {
        uint32_t t = i < rhs.data_.size() ? rhs.data_[i] : 0;
        uint64_t temp = static_cast<uint64_t>(data_[i]) + t + of;
        data_[i] = temp;
        of = temp > UINT32_MAX;
    }
    if (of) {
        data_.push_back(of);
    }
    remove_zeros();
    return *this;
}

big_integer& big_integer::operator-=(big_integer const& rhs) {
    if (sign) {
        *this = rhs.sign ? (-rhs - -*this) : -(-*this + rhs);
        return *this;
    } else {
        if (rhs.sign) {
            *this += -rhs;
            return *this;
        }
    }
    if (*this < rhs) {
        *this = -(rhs - *this);
        return *this;
    }
    bool carry = false;
    for (size_t i = 0; i < data_.size(); ++i) {
        uint32_t t = rhs.data_.size() > i ? rhs.data_[i] : 0;
        if (data_[i] < carry + t) {
            data_[i] = static_cast<uint64_t>(UINT32_MAX) - t - carry + data_[i] + 1;
            carry = true;
        } else {
            data_[i] -= carry + t;
            carry = false;
        }
    }
    remove_zeros();
    return *this;
}

big_integer& big_integer::operator*=(big_integer const& rhs) {
    big_integer res;
    res.data_.resize(data_.size() + rhs.data_.size(), 0);
    res.sign = sign ^ rhs.sign;
    uint32_t of = 0;
    for (size_t i = 0; i < data_.size(); ++i) {
        for (size_t j = 0; j < rhs.data_.size() || of > 0; ++j) {
            uint64_t temp = res.data_[i + j] + static_cast<uint64_t>(data_[i]) * (j < rhs.data_.size() ? rhs.data_[j] : 0) + of;
            of = overflow(temp);
            res.data_[i + j] = temp;
        }
    }
    *this = res;
    remove_zeros();
    return *this;
}

uint64_t shift(uint32_t a) {
    return static_cast<uint64_t>(a) << 32u;
}

big_integer& big_integer::div_by_short(uint32_t a) {
    big_integer res;
    res.data_.resize(data_.size());
    uint32_t carry = 0;
    for (size_t i = data_.size(); i > 0; --i) {
        uint64_t temp = data_[i - 1] + shift(carry);
        res.data_[i - 1] = temp / a;
        carry = temp % a;
    }
    *this = res;
    remove_zeros();
    return *this;
}

big_integer& big_integer::operator/=(big_integer const& rhs) {
    if (abs(*this) < abs(rhs)) {
        *this = 0;
        return *this;
    }
    if (sign ^ rhs.sign) {
        *this = sign ? -(-*this / rhs) : -(*this / -rhs);
        return *this;
    }
    if (rhs.data_.size() == 1) {
        *this = div_by_short(rhs.data_[0]);
        return *this;
    }
    size_t n = data_.size(), m = rhs.data_.size();
    uint32_t f = BASE / (static_cast<uint64_t>(rhs.data_[m - 1]) + 1);
    big_integer r = *this * f;
    big_integer d = rhs * f;
    big_integer q;
    q.data_.resize(n - m + 1);
    r.data_.push_back(0);
    for (ptrdiff_t k = n - m; k >= 0; --k) {
        uint32_t qt = r.trial(d, k, m);
        big_integer dq = d * qt;
        dq.data_.resize(dq.data_.size() + m + 1, 0);
        if (r.smaller(dq, k, m)) {
            --qt;
            dq = big_integer(d) * qt;
        }
        q.data_[k] = qt;
        r.difference(dq, k, m);
    }
    q.remove_zeros();
    *this = q;
    return *this;
}

big_integer& big_integer::operator%=(big_integer const& rhs) {
    *this = *this - (*this / rhs) * rhs;
    return *this;
}

big_integer big_integer::operator+() const {
    return *this;
}

big_integer big_integer::operator-() const {
    big_integer res(*this);
    if (res != 0) {
        res.sign ^= true;
    }
    return res;
}

big_integer operator+(big_integer a, big_integer const& b) {
    return a += b;
}

big_integer operator-(big_integer a, big_integer const& b) {
    return a -= b;
}

big_integer operator*(big_integer a, big_integer const& b) {
    return a *= b;
}

big_integer operator/(big_integer a, big_integer const& b) {
    return a /= b;
}

big_integer operator%(big_integer a, big_integer const& b) {
    return a %= b;
}

big_integer& big_integer::operator++() {
    *this += 1;
    return *this;
}

big_integer big_integer::operator++(int) {
    big_integer r = *this;
    ++*this;
    return r;
}

big_integer& big_integer::operator--() {
    *this -= 1;
    return *this;
}

big_integer big_integer::operator--(int) {
    big_integer r = *this;
    --*this;
    return r;
}

std::string to_string(big_integer const& a) {
    if (a == ZERO) {
        return "0";
    }
    std::string res;
    big_integer temp(a);
    while (temp != 0) {
        std::string t = std::to_string((temp % BASE_STRING).data_[0]);
        std::reverse(t.begin(), t.end());
        if (temp.data_.size() > 1) {
            while (t.size() < STEP) {
                t.push_back('0');
            }
        }
        res += t;
        temp /= BASE_STRING;
    }
    if (a.sign) {
        res.push_back('-');
    }
    std::reverse(res.begin(), res.end());
    return res;
}

void big_integer::remove_zeros() {
    while (data_.size() > 1 && data_.back() == 0) {
        data_.pop_back();
    }
    if (sign && data_.size() == 1 && data_[0] == 0) {
        sign = false;
    }
}

big_integer big_integer::abs(big_integer a) {
    a.sign = false;
    return a;
}

std::ostream& operator<<(std::ostream& s, big_integer const& a) {
    return s << to_string(a);
}

big_integer big_integer::operator~() const {
    big_integer r = -(*this + 1);
    return r;
}

void big_integer::to_addition_two() {
    if (sign) {
        for (size_t i = 0; i < data_.size(); ++i) {
            data_[i] = ~data_[i];
        }
        --*this;
    }
}

big_integer bit_operation(big_integer a, big_integer b, uint32_t func(uint32_t, uint32_t)) {
    size_t size = std::max(a.data_.size(), b.data_.size());
    a.data_.resize(size, 0);
    b.data_.resize(size, 0);
    a.to_addition_two();
    b.to_addition_two();
    big_integer res;
    res.data_.resize(size);
    res.sign = func(a.sign, b.sign);
    for (size_t i = 0; i < size; ++i) {
        res.data_[i] = func(a.data_[i], b.data_[i]);
    }
    res.to_addition_two();
    res.remove_zeros();
    return res;
}

big_integer& big_integer::operator&=(big_integer const& rhs) {
    *this = bit_operation(*this, rhs, [](uint32_t a, uint32_t b) {
        return a & b;
    });
    return *this;
}

big_integer& big_integer::operator|=(big_integer const& rhs) {
    *this = bit_operation(*this, rhs, [](uint32_t a, uint32_t b) {
        return a | b;
    });
    return *this;
}

big_integer& big_integer::operator^=(big_integer const& rhs) {
    *this = bit_operation(*this, rhs, [](uint32_t a, uint32_t b) {
        return a ^ b;
    });
    return *this;
}

big_integer operator&(big_integer a, big_integer const& b) {
    return a &= b;
}

big_integer operator|(big_integer a, big_integer const& b) {
    return a |= b;
}

big_integer operator^(big_integer a, big_integer const& b) {
    return a ^= b;
}

big_integer& big_integer::operator<<=(int rhs) {
    size_t t = rhs % 32;
    size_t add_zeros = rhs >> 5;
    data_.resize(data_.size() + add_zeros);
    for (size_t i = data_.size(); i > add_zeros; --i) {
        data_[i - 1] = data_[i - 1 - add_zeros];
    }
    for (size_t i = 0; i < add_zeros; ++i) {
        data_[i] = 0;
    }
    uint32_t of = 0;
    for (size_t i = add_zeros; i < data_.size(); ++i) {
        uint64_t temp = (static_cast<uint64_t>(data_[i]) << t) + of;
        of = overflow(temp);
        data_[i] = temp;
    }
    if (of) {
        data_.push_back(of);
    }
    return *this;
}

big_integer& big_integer::operator>>=(int rhs) {
    size_t t = rhs % 32;
    size_t remove_digit = rhs >> 5;
    for (size_t i = 0; i < (data_.size() - remove_digit); ++i) {
        data_[i] = data_[i + remove_digit];
    }
    for (size_t i = 0; i < remove_digit; ++i) {
        data_.pop_back();
    }
    uint32_t of = 0;
    for (size_t i = data_.size(); i > 0; --i) {
        uint64_t temp = static_cast<uint64_t>(data_[i - 1]) << (32 - t);
        data_[i - 1] = overflow(temp) + of;
        of = temp;
    }
    if (sign) {
        --*this;
    }
    remove_zeros();
    return *this;
}

uint32_t big_integer::trial(big_integer &d, size_t k, size_t m) {
    size_t km = k + m;
    uint128 r3 = (data_[km] * BASE + data_[km - 1]) * BASE + data_[km - 2];
    uint64_t d2 = d.data_[m - 1] * BASE + d.data_[m - 2];
    return std::min(r3 / d2, BASE - 1);
}


bool big_integer::smaller(big_integer &dq, size_t k, size_t m) {
    size_t i = m, j = 0;
    while (i != j) {
        if (data_[i + k] != dq.data_[i]) {
            j = i;
        } else {
            --i;
        }
    }
    return data_[i + k] < dq.data_[i];
}

void big_integer::difference(big_integer &dq, size_t k, size_t m) {
    uint32_t borrow = 0;
    for (size_t i = 0; i <= m; ++i) {
        uint64_t diff = BASE + data_[i + k] - dq.data_[i] - borrow;
        data_[i + k] = diff;
        borrow = 1 - overflow(diff);
    }
}

big_integer operator<<(big_integer a, int b) {
    return a <<= b;
}

big_integer operator>>(big_integer a, int b) {
    return a >>= b;
}
