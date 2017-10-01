#include "state.hpp"

#include "bit_manipulations.hpp"

namespace bm = bit_manipulations;

namespace state {

// movable generator
// ref: 

board mobility_backward_p2(board bd1, board bd2) {
  __m128i b = _mm_unpacklo_epi64(bd1, bd2);
  __m128i b1 = _mm_add_epi8(b, b);
  __m128i w = _mm_unpackhi_epi64(bd1, bd2);
  return _mm_andnot_si128(_mm_or_si128(b1, w), _mm_add_epi8(b1, w));
}

double_board flippable_right_p4(double_board dbd1, double_board dbd2) {
	__m256i b = _mm256_unpacklo_epi64(dbd1, dbd2);
	__m256i b1 = _mm256_add_epi8(b, b);
	__m256i w = _mm256_unpackhi_epi64(dbd1, dbd2);
	return _mm256_andnot_si256(_mm256_or_si256(b1, w), _mm256_add_epi8(b1, w));
}

uint64_t mobility_horizontal(const board &bd) {
  board tmp = mobility_backward_p2(bd, bm::mirrorHorizontal(bd));
  return tmp.player() | bm::mirrorHorizontal(tmp.opponent());
}

uint64_t mobility_horizontal_and_vertical(const board &bd) {
  board bd_h = bm::flipDiagA1H8(bd);
  double_board dbd1(bd, bd_h);
  double_board dbd2 = bm::mirrorHorizontal(dbd1);
  double_board res = flippable_right_p4(dbd1, dbd2);
  double_board resp = _mm256_permute4x64_epi64(res, 0b11011000);
  board res1 = resp.board1();
  board res2 = bm::mirrorHorizontal(resp.board2());
  board resb = _mm_or_si128(res1, res2);
  return resb.player() | bm::flipDiagA1H8(resb.opponent());
}

uint64_t mobility_diag_implA8H1(const board &bd) {
  const board prot45_bd = bm::pseudoRotate45clockwise(bd);
  uint64_t mask64 = UINT64_C(0x80C0E0F0F8FCFEFF);
  __m128i mask = _mm_set1_epi64x(mask64);
  uint64_t res = (mask64 & mobility_horizontal(_mm_and_si128(mask, prot45_bd))) |
      (~mask64 & mobility_horizontal(_mm_andnot_si128(mask, prot45_bd)));
  return bm::pseudoRotate45antiClockwise(res);
}

uint64_t mobility_diag_implA1H8(const board &bd) {
  const board prot45a_bd = bm::pseudoRotate45antiClockwise(bd);
  uint64_t mask64 = UINT64_C(0xFEFCF8F0E0C08000);
  __m128i mask = _mm_set1_epi64x(mask64);
  uint64_t res = (mask64 & mobility_horizontal(_mm_and_si128(mask, prot45a_bd))) |
      (~mask64 & mobility_horizontal(_mm_andnot_si128(mask, prot45a_bd)));
  return bm::pseudoRotate45clockwise(res);
}

uint64_t mobility_diag(const board &bd) {
  return bm::rotr(
      mobility_diag_implA8H1(bd) |
      mobility_diag_implA1H8(bd), 8);
}

uint64_t mobility_pos(const board &bd) {
  return (mobility_horizontal_and_vertical(bd) |
      mobility_diag(bd)) & ~bm::stones(bd);
}

int mobility_count(const board &bd) {
  return _popcnt64(mobility_pos(bd));
}

} // namespace state
