#include "state.hpp"

#include <cassert>

#include <iomanip>
#include <iostream>

#include "bit_manipulations.hpp"

#include "immintrin.h"

namespace bm = bit_manipulations;

namespace state {

enum class state {
  NONE,
  BLACK,
  WHITE
};

void init() {
}


bool puttable_black_at_dir(const board & bd,
    const int i, const int j, const int dir) {
  const int di[8] = {1, 1, 0, -1, -1, -1, 0, 1};
  const int dj[8] = {0, 1, 1, 1, 0, -1, -1, -1};
  for (int k = 1; k < 8; ++k) {
    int ni = i + di[dir] * k;
    int nj = j + dj[dir] * k;
    if (ni < 0 || nj < 0 || ni >= 8 || nj >= 8) return false;
    if (bd.black().get(ni*8+nj)) {
      return k >= 2;
    } else if (!bd.white().get(ni*8+nj)) {
      return false;
    }
  }
  return false;
}

bool puttable_black_at(const board & bd, const int i, const int j) {
  if (bd.black().get(i*8+j) || bd.white().get(i*8+j)) return false;
  for (int dir = 0; dir < 8; ++dir)
    if (puttable_black_at_dir(bd, i, j, dir)) return true;
  return false;
}

uint64_t puttable_black_naive(const board & bd) {
  uint64_t res = 0;
  for (int i = 0; i < 8; ++i)
    for (int j = 0; j < 8; ++j)
      if (puttable_black_at(bd, i, j))
        res |= ((uint64_t)1 << (i * 8 + j));
  return res;
}

// puttable_black
uint64_t puttable_black_horizontal(const board &bd) {
  board tmp = bm::puttable_black_backward_p2(bd, bm::mirrorHorizontal(bd));
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

bool is_gameover(const board &bd) {
  return puttable_black(bd) == 0 &&
      puttable_black(board::reverse_board(bd)) == 0;
}

/*
void put_black_at_dir(board &bd, int i, int j, int dir) {
  const int di[8] = {1, 1, 0, -1, -1, -1, 0, 1};
  const int dj[8] = {0, 1, 1, 1, 0, -1, -1, -1};
  for (int k = 1; k < 8; ++k) {
    int ni = i + di[dir] * k;
    int nj = j + dj[dir] * k;
    if (ni < 0 || nj < 0 || ni >= 8 || nj >= 8) return;
    if (bd.black().get(ni*8+nj)) {
      for (int l = 1; l < k; ++l) {
        int li = i + di[dir] * l;
        int lj = j + dj[dir] * l;
        bd.black().set(li*8+lj);
        bd.white().reset(li*8+lj);
      }
      return;
    } else if (!bd.white().get(ni*8+nj)) {
      return;
    }
  }
}

board put_black_at_naive(const board & bd, int i, int j) {
  board res = bd;
  for (int dir = 0; dir < 8; ++dir)
    put_black_at_dir(res, i, j, dir);
  res.black().set(i*8+j);
  return res;
}
*/

struct u64_4 {
  __m256i data;
  u64_4() = default;
  u64_4(uint64_t val)
    : data(_mm256_set1_epi64x(val)) {}
  u64_4(uint64_t x, uint64_t y, uint64_t z, uint64_t w)
    : data(_mm256_set_epi64x(x, y, z, w)) {}
  u64_4(__m256i data) : data(data) {}
};

u64_4 operator>>(const u64_4 lhs, const size_t n) {
  return _mm256_srli_epi64(lhs.data, n);
}

u64_4 operator<<(const u64_4 lhs, const size_t n) {
  return _mm256_slli_epi64(lhs.data, n);
}

u64_4 operator&(const u64_4 lhs, const u64_4 rhs) {
  return _mm256_and_si256(lhs.data, rhs.data);
}

u64_4 operator&(const u64_4 lhs, const uint64_t rhs) {
  __m256i r64 = _mm256_set1_epi64x(rhs);
  return _mm256_and_si256(lhs.data, r64);
}

u64_4 operator|(const u64_4 lhs, const u64_4 rhs) {
  return _mm256_or_si256(lhs.data, rhs.data);
}

u64_4 operator|(const u64_4 lhs, const uint64_t rhs) {
  __m256i r64 = _mm256_set1_epi64x(rhs);
  return _mm256_or_si256(lhs.data, r64);
}

u64_4 operator+(const u64_4 lhs, const u64_4 rhs) {
  return _mm256_add_epi64(lhs.data, rhs.data);
}

u64_4 operator+(const u64_4 lhs, const uint64_t rhs) {
  __m256i r64 = _mm256_set1_epi64x(rhs);
  return _mm256_add_epi64(lhs.data, r64);
}

u64_4 operator-(const u64_4 lhs, const u64_4 rhs) {
  return _mm256_sub_epi64(lhs.data, rhs.data);
}

u64_4 operator!=(const u64_4 lhs, const uint64_t rhs) {
  __m256i r64 = _mm256_set1_epi64x(rhs);
  return _mm256_cmpeq_epi64(lhs.data, r64) + u64_4(1);
}

u64_4 operator-(const u64_4 lhs) {
  return _mm256_sub_epi64(_mm256_setzero_si256(), lhs.data);
}

// (NOT lhs) AND rhs
u64_4 andnot(const u64_4 lhs, const u64_4 rhs) {
  return _mm256_andnot_si256(lhs.data, rhs.data);
}

u64_4 operator~(const u64_4 lhs) {
  return _mm256_andnot_si256(lhs.data, _mm256_set1_epi8(0xFF));
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
  u64_4 flipped, OM, outflank, mask;
  uint64_t yzw = bd.white() & UINT64_C(0x7E7E7E7E7E7E7E7E);
  OM = u64_4(bd.white(), yzw, yzw, yzw);
  mask = {
    UINT64_C(0x0080808080808080),
    UINT64_C(0x7F00000000000000),
    UINT64_C(0x0102040810204000),
    UINT64_C(0x0040201008040201)
  };
  mask = mask >> (63 - pos);
  outflank = upper_bit(andnot(OM, mask)) & bd.black();
  flipped = (-outflank << 1) & mask;
  mask = {
    UINT64_C(0x0101010101010100),
    UINT64_C(0x00000000000000FE),
    UINT64_C(0x0002040810204080),
    UINT64_C(0x8040201008040200)
  };
  mask = mask << pos;
  outflank = mask & ((OM | ~mask) + 1) & bd.black();
  flipped = flipped | ((outflank - (outflank != 0)) & mask);
  __m128i flipped_xz_yw = _mm_or_si128(_mm256_castsi256_si128(flipped.data),
      _mm256_extractf128_si256(flipped.data, 1));
  return _mm_or_si128(flipped_xz_yw, _mm_alignr_epi8(flipped_xz_yw, flipped_xz_yw, 8));
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

std::vector<board> next_states(const board & bd) {
  std::vector<board> res;
  res.reserve(16);
  bool is_pass = true;
  for (uint64_t bits = puttable_black(bd); bits != 0; bits &= bits - 1) {
    int pos = bm::bit_to_pos(bits & -bits);
    res.emplace_back(put_black_at_rev(bd, pos / 8, pos % 8));
    is_pass = false;
  }
  if (is_pass) res.emplace_back(bd, reverse_construct_t());
  return res;
}

void next_states(const board & bd, std::vector<board> &res) {
  res.clear();
  bool is_pass = true;
  for (uint64_t bits = puttable_black(bd); bits != 0; bits &= bits - 1) {
    int pos = bm::bit_to_pos(bits & -bits);
    res.emplace_back(put_black_at_rev(bd, pos / 8, pos % 8));
    is_pass = false;
  }
  if (is_pass) res.emplace_back(bd, reverse_construct_t());
}

std::vector<board> next_states(const board & bd, uint64_t bits) {
  std::vector<board> res;
  res.reserve(16);
  if (bits == 0) res.emplace_back(bd, reverse_construct_t());
  for (; bits != 0; bits &= bits - 1) {
    int pos = bm::bit_to_pos(bits & -bits);
    res.emplace_back(put_black_at_rev(bd, pos / 8, pos % 8));
  }
  return res;
}

void next_states(const board & bd, uint64_t bits, std::vector<board> &res) {
  res.clear();
  if (bits == 0) res.emplace_back(bd, reverse_construct_t());
  for (; bits != 0; bits &= bits - 1) {
    int pos = bm::bit_to_pos(bits & -bits);
    res.emplace_back(put_black_at_rev(bd, pos / 8, pos % 8));
  }
}

} // namespace state
