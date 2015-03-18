#include "bit_manipulations.hpp"

#include <immintrin.h>

namespace bit_manipulations {

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
  bd.data = _mm_add_epi8(
      _mm_and_si128(
          _mm_srli_epi64(bd.data, 1),
          mask1),
      _mm_slli_epi64(
          _mm_and_si128(bd.data, mask1),
          1)
      );
  bd.data = _mm_add_epi8(
      _mm_and_si128(
          _mm_srli_epi64(bd.data, 2),
          mask2),
      _mm_slli_epi64(
          _mm_and_si128(bd.data, mask2),
          2)
      );
  bd.data = _mm_add_epi8(
      _mm_and_si128(
          _mm_srli_epi64(bd.data, 4),
          mask3),
      _mm_slli_epi64(
          _mm_and_si128(bd.data, mask3),
          4)
      );
  return bd;
}

} // namespace bit_manipulations
