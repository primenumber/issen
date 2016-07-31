#pragma once
#include <array>
#include <x86intrin.h>
#include "gcc_intrinsics.hpp"

struct bit_board {
  uint64_t data;
  bit_board() = default;
  constexpr bit_board(const bit_board &) = default;
  constexpr bit_board(bit_board &&) = default;
  constexpr bit_board(const uint64_t data) : data(data) {}
  bit_board &operator=(const bit_board &) = default;
  bit_board &operator=(bit_board &&) = default;
  operator uint64_t() { return data; }
  operator uint64_t() const { return data; }
  bool get(const size_t index) const { return _bittest64(&data, index); }
  bool set(const size_t index) { return _bittestandset64(&data, index); }
  bool reset(const size_t index) { return _bittestandreset64(&data, index); }
  bool setval(const size_t index, const bool bit) {
    return bit ? set(index) : reset(index);
  }
};

struct reverse_construct_t {};

union board {
  __m128i data;
  std::array<bit_board, 2> ary;
  board() = default;
  board(const board &) = default;
  board(const board & bd, const reverse_construct_t) :
      ary{bd.ary[1], bd.ary[0]} {}
  board(const uint64_t black, const uint64_t white) : ary{black, white} {}
  board(__m128i data) : data(data) {}
  operator __m128i() { return data; }
  operator __m128i() const { return data; }
  board &operator=(const board &) = default;
  board &operator=(board &&) = default;
  bit_board &black() { return ary[0]; }
  const bit_board &black() const { return ary[0]; }
  bit_board &white() { return ary[1]; }
  const bit_board &white() const { return ary[1]; }
  static board initial_board() {
    return board(UINT64_C(0x0000000810000000), UINT64_C(0x0000001008000000));
  }
  static board reverse_board(const board &bd) {
    return board(bd, reverse_construct_t());
  }
};

inline bool operator==(const board &lhs, const board &rhs) {
  return lhs.black() == rhs.black() &&
      lhs.white() == rhs.white();
}

inline bool operator<(const board &lhs, const board &rhs) {
  return (lhs.black() == rhs.black()) ?
      (lhs.white() < rhs.white()) : (lhs.black() < rhs.black());
}

namespace std {

template<>
struct hash<board> {
 public:
  size_t operator()(const board &bd) const {
    return bd.black() + bd.white() * 17;
  }
};

} // namespace std
