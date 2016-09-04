#include "state.hpp"

#include "bit_manipulations.hpp"

namespace bm = bit_manipulations;

namespace state {

// movable generator
// ref: 

board puttable_black_backward_p2(board bd1, board bd2) {
  __m128i b = _mm_unpacklo_epi64(bd1, bd2);
  __m128i b1 = _mm_add_epi8(b, b);
  __m128i w = _mm_unpackhi_epi64(bd1, bd2);
  return _mm_andnot_si128(_mm_or_si128(b1, w), _mm_add_epi8(b1, w));
}

uint64_t puttable_black_horizontal(const board &bd) {
  board tmp = puttable_black_backward_p2(bd, bm::mirrorHorizontal(bd));
  return tmp.black() | bm::mirrorHorizontal(tmp.white());
}

uint64_t puttable_black_vertical(const board &bd) {
  return bm::flipDiagA1H8(puttable_black_horizontal(bm::flipDiagA1H8(bd)));
}

uint64_t puttable_black_diag_implA8H1(const board &bd) {
  const board prot45_bd = bm::pseudoRotate45clockwise(bd);
  uint64_t mask64 = UINT64_C(0x80C0E0F0F8FCFEFF);
  __m128i mask = _mm_set1_epi64x(mask64);
  uint64_t res = (mask64 & puttable_black_horizontal(_mm_and_si128(mask, prot45_bd))) |
      (~mask64 & puttable_black_horizontal(_mm_andnot_si128(mask, prot45_bd)));
  return bm::pseudoRotate45antiClockwise(res);
}

uint64_t puttable_black_diag_implA1H8(const board &bd) {
  const board prot45a_bd = bm::pseudoRotate45antiClockwise(bd);
  uint64_t mask64 = UINT64_C(0xFEFCF8F0E0C08000);
  __m128i mask = _mm_set1_epi64x(mask64);
  uint64_t res = (mask64 & puttable_black_horizontal(_mm_and_si128(mask, prot45a_bd))) |
      (~mask64 & puttable_black_horizontal(_mm_andnot_si128(mask, prot45a_bd)));
  return bm::pseudoRotate45clockwise(res);
}

uint64_t puttable_black_diag(const board &bd) {
  return bm::rotr(
      puttable_black_diag_implA8H1(bd) |
      puttable_black_diag_implA1H8(bd), 8);
}

uint64_t puttable_black(const board &bd) {
  return (puttable_black_horizontal(bd) |
      puttable_black_vertical(bd) |
      puttable_black_diag(bd)) & ~bm::stones(bd);
}

} // namespace state
