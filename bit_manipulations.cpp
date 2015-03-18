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

} // namespace bit_manipulations
