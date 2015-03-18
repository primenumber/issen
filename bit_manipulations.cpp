#include "bit_manipulations.hpp"

#include <immintrin.h>

namespace bit_manipulations {

__m128i operator^(__m128i lhs, __m128i rhs) {
  return _mm_xor_si128(lhs, rhs);
}

__m128i operator&(__m128i lhs, __m128i rhs) {
  return _mm_and_si128(lhs, rhs);
}

__m128i operator|(__m128i lhs, __m128i rhs) {
  return _mm_or_si128(lhs, rhs);
}

__m128i operator<<(__m128i lhs, int index) {
  return _mm_slli_epi64(lhs, index);
}

__m128i operator>>(__m128i lhs, int index) {
  return _mm_srli_epi64(lhs, index);
}

__m128i delta_swap(__m128i bits, __m128i mask, int delta) {
  __m128i tmp = mask & (bits ^ (bits << delta));
  return bits ^ tmp ^ (tmp >> delta);
}

alignas(32) __m128i flip_vertical_shuffle_table;

void init() {
  flip_vertical_shuffle_table = _mm_set_epi8(8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7);
}

board flipVertical(const board bd) {
  return board(_mm_shuffle_epi8(bd.data, flip_vertical_shuffle_table));
}

board mirrorHorizontal(board bd) {
  __m128i mask1 = _mm_set1_epi8(0x55);
  __m128i mask2 = _mm_set1_epi8(0x33);
  __m128i mask3 = _mm_set1_epi8(0x0f);
  bd.data = ((bd.data >> 1) & mask1) | ((bd.data & mask1) << 1);
  bd.data = ((bd.data >> 2) & mask2) | ((bd.data & mask2) << 2);
  bd.data = ((bd.data >> 4) & mask3) | ((bd.data & mask3) << 4);
  return bd;
}

} // namespace bit_manipulations
