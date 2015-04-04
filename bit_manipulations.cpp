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

template <typename T>
T delta_swap(T bits, T mask, int delta) {
  T tmp = mask & (bits ^ (bits << delta));
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

board flipVertical(board bd) {
  return board(_mm_shuffle_epi8(bd.data, flip_vertical_shuffle_table));
}

uint64_t flipVertical(uint64_t bits) {
  return _bswap64(bits);
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

uint64_t mirrorHorizontal(uint64_t bits) {
  uint64_t mask1 = UINT64_C(0x5555555555555555);
  uint64_t mask2 = UINT64_C(0x3333333333333333);
  uint64_t mask3 = UINT64_C(0x0f0f0f0f0f0f0f0f);
  bits = ((bits >> 1) & mask1) | ((bits & mask1) << 1);
  bits = ((bits >> 2) & mask2) | ((bits & mask2) << 2);
  bits = ((bits >> 4) & mask3) | ((bits & mask3) << 4);
  return bits;
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

uint64_t flipDiagA1H8(uint64_t bits) {
  uint64_t mask1 = UINT64_C(0x5500550055005500);
  uint64_t mask2 = UINT64_C(0x3333000033330000);
  uint64_t mask3 = UINT64_C(0x0f0f0f0f00000000);
  bits = delta_swap(bits, mask3, 28);
  bits = delta_swap(bits, mask2, 14);
  return delta_swap(bits, mask1, 7);
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

uint64_t flipDiagA8H1(uint64_t bits) {
  uint64_t mask1 = UINT64_C(0xaa00aa00aa00aa00);
  uint64_t mask2 = UINT64_C(0xcccc0000cccc0000);
  uint64_t mask3 = UINT64_C(0xf0f0f0f000000000);
  bits = delta_swap(bits, mask3, 36);
  bits = delta_swap(bits, mask2, 18);
  return delta_swap(bits, mask1, 9);
}

board rotate180(board bd) {
  return mirrorHorizontal(flipVertical(bd));
}

uint64_t rotate180(uint64_t bits) {
  return mirrorHorizontal(flipVertical(bits));
};

board rotate90clockwise(board bd) {
  return flipVertical(flipDiagA8H1(bd));
}

uint64_t rotate90clockwise(uint64_t bits) {
  return flipVertical(flipDiagA8H1(bits));
}

board rotate90antiClockwise(board bd) {
  return flipVertical(flipDiagA1H8(bd));
}

uint64_t rotate90antiClockwise(uint64_t bits) {
  return flipVertical(flipDiagA1H8(bits));
}

__m128i rotr(__m128i bits, int index) {
  board bd(bits);
  return board(_lrotr(bd.black.data, index),
      _lrotr(bd.white.data, index)).data;
}

uint64_t rotr(uint64_t bits, int index) {
  return _lrotr(bits, index);
}

board pseudoRotate45clockwise(board bd) {
  __m128i mask1 = _mm_set1_epi8(0x55);
  __m128i mask2 = _mm_set1_epi8(0x33);
  __m128i mask3 = _mm_set1_epi8(0x0f);
  __m128i data = bd.data ^ (mask1 & (bd.data ^ rotr(bd.data, 8)));
  data = data ^ (mask2 & (data ^ rotr(data, 16)));
  return data ^ (mask3 & (data ^ rotr(data, 32)));
}

uint64_t pseudoRotate45clockwise(uint64_t bits) {
  uint64_t mask1 = UINT64_C(0x5555555555555555);
  uint64_t mask2 = UINT64_C(0x3333333333333333);
  uint64_t mask3 = UINT64_C(0x0f0f0f0f0f0f0f0f);
  bits = bits ^ (mask1 & (bits ^ rotr(bits, 8)));
  bits = bits ^ (mask2 & (bits ^ rotr(bits, 16)));
  return bits ^ (mask3 & (bits ^ rotr(bits, 32)));
}

board pseudoRotate45antiClockwise(board bd) {
  __m128i mask1 = _mm_set1_epi8(0xaa);
  __m128i mask2 = _mm_set1_epi8(0xcc);
  __m128i mask3 = _mm_set1_epi8(0xf0);
  __m128i data = bd.data ^ (mask1 & (bd.data ^ rotr(bd.data, 8)));
  data = data ^ (mask2 & (data ^ rotr(data, 16)));
  return data ^ (mask3 & (data ^ rotr(data, 32)));
}

uint64_t pseudoRotate45antiClockwise(uint64_t bits) {
  uint64_t mask1 = UINT64_C(0xaaaaaaaaaaaaaaaa);
  uint64_t mask2 = UINT64_C(0xcccccccccccccccc);
  uint64_t mask3 = UINT64_C(0xf0f0f0f0f0f0f0f0);
  bits = bits ^ (mask1 & (bits ^ rotr(bits, 8)));
  bits = bits ^ (mask2 & (bits ^ rotr(bits, 16)));
  return bits ^ (mask3 & (bits ^ rotr(bits, 32)));
}

uint64_t tails(uint64_t bits) {
  ++bits;
  return (bits & -bits) - 1;
}

board tails(board bd) {
  return board(tails(bd.black.data), tails(bd.white.data));
}

board definites_horizontal_top(board bd) {
  return board(tails(bd).data |
      mirrorHorizontal(tails(mirrorHorizontal(bd))).data);
}
board definites_horizontal(board bd) {
  return board(
      definites_horizontal_top(bd).data |
      flipVertical(definites_horizontal_top(flipVertical(bd))).data);
}

board definites(board bd) {
  return board(
      definites_horizontal(bd).data |
      flipDiagA1H8(definites_horizontal(flipDiagA1H8(bd))).data);
}  

uint64_t puttable_black_forward_nomask(board bd) {
  __m128i posbit = _mm_set_epi64x(
          UINT64_C(0x0404040404040404),
          UINT64_C(0x0202020202020202));
  __m128i pres = _mm_setzero_si128();
  for (int i = 0; i < 3; ++i) {
    __m128i b = _mm_set1_epi64x(bd.black.data);
    __m128i w = _mm_set1_epi64x(bd.white.data);
    __m128i wpp = _mm_add_epi8(w, posbit);
    __m128i poyo = _mm_subs_epu8(b & wpp, posbit);
    pres = pres | (poyo & posbit);
    posbit = posbit << 2;
  }
  pres = pres | _mm_srli_si128(pres, 8);
  return _mm_cvtsi128_si64(pres) >> 1;
}

uint64_t puttable_black_forward(board bd) {
  return puttable_black_forward_nomask(bd) & ~(bd.black.data | bd.white.data);
}

int bit_to_pos(uint64_t bit) {
  return _popcnt64(bit - 1);
}

uint16_t toBase3(uint8_t black, uint8_t white) {
  return base3[black] + 2*base3[white];
}

} // namespace bit_manipulations
