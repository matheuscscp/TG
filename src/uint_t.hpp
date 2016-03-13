#ifndef UINT_T_HPP
#define UINT_T_HPP

#include <vector>
#include <cstdint>
#include <ostream>

struct uint_t {
  int n;
  std::vector<uint8_t> buf;
  uint_t();
  uint_t(unsigned);
  uint_t& operator=(unsigned);
  operator bool();
  int get(int) const;
  void set(int);
  bool operator<(const uint_t&) const;
  uint_t& swap(uint_t&);
  uint_t operator+(const uint_t&) const;
  uint_t& operator+=(const uint_t&);
  uint_t operator*(const uint_t&) const;
  uint_t& operator*=(const uint_t&);
};
std::ostream& operator<<(std::ostream&, const uint_t&);

#endif
