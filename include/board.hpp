#pragma once
#include <array>
#include <x86intrin.h>
#include "gcc_intrinsics.hpp"

struct half_board {
  uint64_t data;
  half_board() = default;
  constexpr half_board(const half_board &) = default;
  constexpr half_board(half_board &&) = default;
  constexpr half_board(const uint64_t data) : data(data) {}
  half_board &operator=(const half_board &) = default;
  half_board &operator=(half_board &&) = default;
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

struct board {
  __m128i data;
  board() = default;
  board(const board &) = default;
  board(const board & bd, const reverse_construct_t)
    : data(_mm_alignr_epi8(bd.data, bd.data, 8)) {}
  board(const uint64_t black, const uint64_t white)
    : data(_mm_setr_epi64x(black, white)) {}
  board(__m128i data) : data(data) {}
  operator __m128i() { return data; }
  operator __m128i() const { return data; }
  board &operator=(const board &) = default;
  board &operator=(board &&) = default;
  const half_board black() const { return _mm_cvtsi128_si64(data); }
  const half_board white() const {
    return _mm_extract_epi64(data, 1);
  }
  static board initial_board() {
    return board(UINT64_C(0x0000000810000000), UINT64_C(0x0000001008000000));
  }
  static board reverse_board(const board &bd) {
    return board(bd, reverse_construct_t());
  }
};

namespace std {

template<>
struct hash<board> {
 public:
  size_t operator()(const board &bd) const {
    return bd.black() + bd.white() * 17;
  }
};

} // namespace std
