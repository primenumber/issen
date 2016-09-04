#include "state.hpp"

#include "bit_manipulations.hpp"

namespace bm = bit_manipulations;

namespace state {

// move generator
// ref: http://www.amy.hi-ho.ne.jp/okuhara/flipcuda.htm

struct u64_4 {
  __m256i data;
  u64_4() = default;
  u64_4(uint64_t val)
    : data(_mm256_set1_epi64x(val)) {}
  u64_4(uint64_t x, uint64_t y, uint64_t z, uint64_t w)
    : data(_mm256_set_epi64x(x, y, z, w)) {}
  u64_4(__m256i data) : data(data) {}
};

inline u64_4 operator>>(const u64_4 lhs, const size_t n) {
  return _mm256_srli_epi64(lhs.data, n);
}

inline u64_4 operator<<(const u64_4 lhs, const size_t n) {
  return _mm256_slli_epi64(lhs.data, n);
}

inline u64_4 operator&(const u64_4 lhs, const u64_4 rhs) {
  return _mm256_and_si256(lhs.data, rhs.data);
}

inline u64_4 operator&(const u64_4 lhs, const uint64_t rhs) {
  __m256i r64 = _mm256_set1_epi64x(rhs);
  return _mm256_and_si256(lhs.data, r64);
}

inline u64_4 operator|(const u64_4 lhs, const u64_4 rhs) {
  return _mm256_or_si256(lhs.data, rhs.data);
}

inline u64_4 operator|(const u64_4 lhs, const uint64_t rhs) {
  __m256i r64 = _mm256_set1_epi64x(rhs);
  return _mm256_or_si256(lhs.data, r64);
}

inline u64_4 operator+(const u64_4 lhs, const u64_4 rhs) {
  return _mm256_add_epi64(lhs.data, rhs.data);
}

inline u64_4 operator+(const u64_4 lhs, const uint64_t rhs) {
  __m256i r64 = _mm256_set1_epi64x(rhs);
  return _mm256_add_epi64(lhs.data, r64);
}

inline u64_4 operator-(const u64_4 lhs, const u64_4 rhs) {
  return _mm256_sub_epi64(lhs.data, rhs.data);
}

inline u64_4 operator!=(const u64_4 lhs, const uint64_t rhs) {
  __m256i r64 = _mm256_set1_epi64x(rhs);
  return _mm256_cmpeq_epi64(lhs.data, r64) + u64_4(1);
}

inline u64_4 operator-(const u64_4 lhs) {
  return _mm256_sub_epi64(_mm256_setzero_si256(), lhs.data);
}

// (NOT lhs) AND rhs
inline u64_4 andnot(const u64_4 lhs, const u64_4 rhs) {
  return _mm256_andnot_si256(lhs.data, rhs.data);
}

inline u64_4 operator~(const u64_4 lhs) {
  return _mm256_andnot_si256(lhs.data, _mm256_set1_epi8(0xFF));
}

__m128i hor(const u64_4 lhs) {
  __m128i lhs_xz_yw = _mm_or_si128(_mm256_castsi256_si128(lhs.data),
      _mm256_extractf128_si256(lhs.data, 1));
  return _mm_or_si128(lhs_xz_yw, _mm_alignr_epi8(lhs_xz_yw, lhs_xz_yw, 8));
}

u64_4 upper_bit(u64_4 p) {
  p = p | (p >> 1);
  p = p | (p >> 2);
  p = p | (p >> 4);
  p = p | (p >> 8);
  p = p | (p >> 16);
  p = p | (p >> 32);
  return andnot(p >> 1, p);
}

__m128i flip(const board &bd, int pos) {
  uint64_t black = bd.black(), white = bd.white();
  u64_4 flipped, OM, outflank, mask;
  uint64_t yzw = white & UINT64_C(0x7E7E7E7E7E7E7E7E);
  OM = u64_4(white, yzw, yzw, yzw);
  mask = {
    UINT64_C(0x0080808080808080),
    UINT64_C(0x7F00000000000000),
    UINT64_C(0x0102040810204000),
    UINT64_C(0x0040201008040201)
  };
  mask = mask >> (63 - pos);
  outflank = upper_bit(andnot(OM, mask)) & black;
  flipped = (-outflank << 1) & mask;
  mask = {
    UINT64_C(0x0101010101010100),
    UINT64_C(0x00000000000000FE),
    UINT64_C(0x0002040810204080),
    UINT64_C(0x8040201008040200)
  };
  mask = mask << pos;
  outflank = mask & ((OM | ~mask) + 1) & black;
  flipped = flipped | ((outflank - (outflank != 0)) & mask);
  return hor(flipped);
}

board put_black_at(const board & bd, int pos) {
  __m128i bits = flip(bd, pos);
  return _mm_or_si128(_mm_xor_si128(bd, bits),
      _mm_set_epi64x(0, UINT64_C(1) << pos));
}

board put_black_at(const board & bd, int i, int j) {
  return put_black_at(bd, i*8 + j);
}

board put_black_at_rev(const board & bd, int pos) {
  __m128i bits = flip(bd, pos);
  return _mm_or_si128(_mm_xor_si128(board::reverse_board(bd), bits),
      _mm_set_epi64x(UINT64_C(1) << pos, 0));
}

board put_black_at_rev(const board & bd, int i, int j) {
  return put_black_at_rev(bd, i*8 + j);
}

} // namespace state
