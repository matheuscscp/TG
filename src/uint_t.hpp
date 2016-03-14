#ifndef UINT_T_HPP
#define UINT_T_HPP

#include <vector>
#include <cstdint>
#include <ostream>

class uint_t {
  private:
    int n;
    std::vector<uint8_t> buf;
  public:
    uint_t();
    uint_t(unsigned);
    uint_t& operator=(unsigned);
    operator bool() const;
    bool operator<(const uint_t&) const;
    uint_t operator+(const uint_t&) const;
    uint_t& operator+=(const uint_t&);
    uint_t operator*(const uint_t&) const;
    uint_t& operator*=(const uint_t&);
    void swap(uint_t&);
    uint_t& times2();
    void output(std::ostream&) const;
  private:
    int get(int) const;
    void set(int);
};
std::ostream& operator<<(std::ostream&, const uint_t&);

#endif
