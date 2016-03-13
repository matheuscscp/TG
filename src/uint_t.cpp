#include <utility>

#include "uint_t.hpp"

using namespace std;

uint_t::uint_t() : n(0), buf(1,0) {}

uint_t::uint_t(unsigned val) {
  operator=(val);
}

uint_t& uint_t::operator=(unsigned val) {
  if (!val) {
    n = 0;
    buf = vector<uint8_t>(1,0);
    return *this;
  }
  int i;
  for (i = (sizeof(unsigned)<<3)-1; 0 <= i; i--) if ((val>>i)&1) break;
  n = i+1;
  buf = vector<uint8_t>((n>>3)+1,0);
  for (int i = 0; i < n; i++) if ((val>>i)&1) set(i);
  return *this;
}

uint_t::operator bool() {
  return n > 0;
}

int uint_t::get(int i) const {
  return (buf[i>>3]>>(i&7))&1;
}

void uint_t::set(int i) {
  buf[i>>3] |= (1<<(i&7));
}

bool uint_t::operator<(const uint_t& b) const {
  if (n != b.n) return n < b.n;
  for (int i = n-2; 0 <= i; i--) if (get(i) != b.get(i)) return b.get(i);
  return false;
}

uint_t& uint_t::swap(uint_t& b) {
  std::swap(n,b.n);
  buf.swap(b.buf);
  return *this;
}

uint_t uint_t::operator+(const uint_t& b) const {
  uint_t ret;
  ret.n = max(n,b.n);
  ret.buf = vector<uint8_t>(((ret.n+1)>>3)+1,0);
  int carry = 0;
  for (int i = 0, ai, bi, sum; i < ret.n; i++) {
    ai = 0, bi = 0;
    if (i <   n) ai =   get(i); else if (!carry) break;
    if (i < b.n) bi = b.get(i); else if (!carry) break;
    sum = ai+bi+carry;
    if (sum&1) ret.set(i);
    carry = sum>>1;
  }
  if (carry) { ret.set(ret.n); ret.n++; }
  while ((ret.n>>3)+1 < ret.buf.size()) ret.buf.pop_back();
  return ret;
}

uint_t& uint_t::operator+=(const uint_t& b) {
  uint_t tmp = this->operator+(b);
  return swap(tmp);
}

uint_t uint_t::operator*(const uint_t& b) const {
  if (!n || !b.n) return uint_t(0);
  bool less = this->operator<(b);
  auto& power = (less ? *this : b);
  uint_t base = (less ? b : *this);
  uint_t ret = 0;
  for (int i = 0; i < power.n; i++) {
    if (power.get(i)) ret += base;
    base += base;
  }
  return ret;
}

uint_t& uint_t::operator*=(const uint_t& b) {
  uint_t tmp = this->operator*(b);
  return swap(tmp);
}

ostream& operator<<(ostream& os, const uint_t& num) {
  if (num.n > 64) {
    os << "2^" << num.n;
    return os;
  }
  uint64_t tmp = 0;
  for (int i = 0; i < num.n; i++) if (num.get(i)) tmp |= (1<<i);
  os << tmp;
  return os;
}
