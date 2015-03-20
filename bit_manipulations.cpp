#include "bit_manipulations.hpp"

#include <immintrin.h>

namespace bit_manipulations {

alignas(32) uint16_t base3[256];

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
  for (int i = 0; i < 256; ++i) {
    int res = 0;
    int pow3 = 1;
    for (int j = 0; j < 8; ++j) {
      res += ((i >> j) & 1) * pow3;
      pow3 *= 3;
    }
    base3[i] = res;
  }
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

board flipDiagA1H8(board bd) {
  __m128i mask1 = _mm_set1_epi16(0x5500);
  __m128i mask2 = _mm_set1_epi32(0x33330000);
  __m128i mask3 = _mm_set1_epi64(
      _mm_cvtsi64_m64(UINT64_C(0x0f0f0f0f00000000)));
  __m128i data = delta_swap(bd.data, mask3, 28);
  data = delta_swap(data, mask2, 14);
  return board(delta_swap(data, mask1, 7));
}

board flipDiagA8H1(board bd) {
  __m128i mask1 = _mm_set1_epi16(0xaa00);
  __m128i mask2 = _mm_set1_epi32(0xcccc0000);
  __m128i mask3 = _mm_set1_epi64(
      _mm_cvtsi64_m64(UINT64_C(0xf0f0f0f000000000)));
  __m128i data = delta_swap(bd.data, mask3, 36);
  data = delta_swap(data, mask2, 18);
  return board(delta_swap(data, mask1, 9));
}

board rotate180(board bd) {
  return mirrorHorizontal(flipVertical(bd));
}

board rotate90clockwise(board bd) {
  return flipVertical(flipDiagA8H1(bd));
}

board rotate90antiClockwise(board bd) {
  return flipVertical(flipDiagA1H8(bd));
}

__m128i rotr(__m128i bits, int index) {
  board bd(bits);
  return board(_lrotr(bd.black.data, index),
      _lrotr(bd.white.data, index)).data;
}

board pseudoRotate45clockwise(board bd) {
  __m128i mask1 = _mm_set1_epi8(0x55);
  __m128i mask2 = _mm_set1_epi8(0x33);
  __m128i mask3 = _mm_set1_epi8(0x0f);
  __m128i data = bd.data ^ (mask1 & (bd.data ^ rotr(bd.data, 8)));
  data = data ^ (mask2 & (data ^ rotr(data, 16)));
  return data ^ (mask3 & (data ^ rotr(data, 32)));
}

uint16_t toBase3(uint8_t black, uint8_t white) {
  return base3[black] + 2*base3[white];
}

} // namespace bit_manipulations
