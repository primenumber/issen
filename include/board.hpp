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

struct board {
  __m128i data;
  board() = default;
  board(const board &) = default;
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
  static board empty_board() {
    return board(_mm_setzero_si128());
  }
  static board reverse_board(const board &bd) {
    return board(_mm_alignr_epi8(bd.data, bd.data, 8));
  }
};

struct double_board {
  __m256i data;
  double_board() = default;
  double_board(const double_board &) = default;
  double_board(const board &bd1, const board &bd2)
    : data(_mm256_setr_m128i(bd1, bd2)) {}
  explicit double_board(const board &bd)
    : data(_mm256_broadcastsi128_si256(bd)) {}
  double_board(const uint64_t black1, const uint64_t white1,
      const uint64_t black2, const uint64_t white2)
    : data(_mm256_setr_epi64x(black1, white1, black2, white2)) {}
  double_board(const __m256i data) : data(data) {}
  operator __m256i() { return data; }
  operator __m256i() const { return data; }
  double_board &operator=(const double_board &) = default;
  double_board &operator=(double_board &&) = default;
  const board board1() const { return _mm256_castsi256_si128(data); }
  const board board2() const { return _mm256_extracti128_si256(data, 1); }
  uint64_t operator[](const size_t index) {
    switch(index) {
      case 0: return _mm256_extract_epi64(data, 0);
      case 1: return _mm256_extract_epi64(data, 1);
      case 2: return _mm256_extract_epi64(data, 2);
      case 3: return _mm256_extract_epi64(data, 3);
      default: return 0;
    }
  }
};

namespace std {

template<>
struct hash<board> {
 public:
  size_t operator()(const board &bd) const {
    return _bswap64(bd.black()) + bd.white() * 17;
  }
};

} // namespace std
