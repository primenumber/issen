#include "state.hpp"

#include <cassert>

#include <iomanip>
#include <iostream>

#include "bit_manipulations.hpp"
#include "line.hpp"

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
  uint64_t res = bm::puttable_black_forward_nomask(bd) |
    bm::mirrorHorizontal(
        bm::puttable_black_forward_nomask(
            bm::mirrorHorizontal(bd)));
  return res;
}

uint64_t puttable_black_vertical(const board &bd) {
  using bm::flipDiagA1H8;
  using bm::flipDiagA8H1;
  using bm::puttable_black_forward_nomask;
  return flipDiagA1H8(puttable_black_forward_nomask(flipDiagA1H8(bd))) |
      flipDiagA8H1(puttable_black_forward_nomask(flipDiagA8H1(bd)));
}

uint64_t puttable_black_diag_implA8H1(const board &bd) {
  const board prot45_bd = bm::pseudoRotate45clockwise(bd);
  uint64_t res = 0;
  for (int i = 0; i < 8; ++i)
    res |= (uint64_t)line::puttable_line(prot45_bd, i, i) << (i * 8);
  return bm::pseudoRotate45antiClockwise(res);
}

uint64_t puttable_black_diag_implA1H8(const board &bd) {
  const board prot45a_bd = bm::pseudoRotate45antiClockwise(bd);
  uint64_t res = 0;
  for (int i = 0; i < 8; ++i)
    res |= (uint64_t)line::puttable_line(prot45a_bd, i, (-i) & 7) << (i * 8);
  return bm::pseudoRotate45clockwise(res);
}

uint64_t puttable_black_diag(const board &bd) {
  return bm::rotr(
      puttable_black_diag_implA8H1(bd) |
      puttable_black_diag_implA1H8(bd), 8);
}

uint64_t puttable_black(const board &bd) {
  return ~(bd.black() | bd.white()) &
      (puttable_black_horizontal(bd) |
      puttable_black_vertical(bd) |
      puttable_black_diag(bd));
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
  __m128i xy, zw;
  u64_4() = default;
  u64_4(uint64_t val)
    : xy(_mm_set1_epi64x(val)), zw(_mm_set1_epi64x(val)) {}
  u64_4(uint64_t x, uint64_t y, uint64_t z, uint64_t w)
    : xy(_mm_set_epi64x(x, y)), zw(_mm_set_epi64x(z, w)) {}
  u64_4(__m128i xy, __m128i zw)
    : xy(xy), zw(zw) {}
};

u64_4 operator>>(const u64_4 lhs, const size_t n) {
  return u64_4(_mm_srli_epi64(lhs.xy, n), _mm_srli_epi64(lhs.zw, n));
}

u64_4 operator<<(const u64_4 lhs, const size_t n) {
  return u64_4(_mm_slli_epi64(lhs.xy, n), _mm_slli_epi64(lhs.zw, n));
}

u64_4 operator&(const u64_4 lhs, const u64_4 rhs) {
  return u64_4(_mm_and_si128(lhs.xy, rhs.xy), _mm_and_si128(lhs.zw, rhs.zw));
}

u64_4 operator&(const u64_4 lhs, const uint64_t rhs) {
  __m128i r64 = _mm_set1_epi64x(rhs);
  return u64_4(_mm_and_si128(lhs.xy, r64), _mm_and_si128(lhs.zw, r64));
}

u64_4 operator|(const u64_4 lhs, const u64_4 rhs) {
  return u64_4(_mm_or_si128(lhs.xy, rhs.xy), _mm_or_si128(lhs.zw, rhs.zw));
}

u64_4 operator|(const u64_4 lhs, const uint64_t rhs) {
  __m128i r64 = _mm_set1_epi64x(rhs);
  return u64_4(_mm_or_si128(lhs.xy, r64), _mm_or_si128(lhs.zw, r64));
}

u64_4 operator+(const u64_4 lhs, const uint64_t rhs) {
  __m128i r64 = _mm_set1_epi64x(rhs);
  return u64_4(_mm_add_epi64(lhs.xy, r64), _mm_add_epi64(lhs.zw, r64));
}

u64_4 operator+(const u64_4 lhs, const u64_4 rhs) {
  return u64_4(_mm_add_epi64(lhs.xy, rhs.xy), _mm_add_epi64(lhs.zw, rhs.zw));
}

u64_4 operator-(const u64_4 lhs, const u64_4 rhs) {
  return u64_4(_mm_sub_epi64(lhs.xy, rhs.xy), _mm_sub_epi64(lhs.zw, rhs.zw));
}

u64_4 operator!=(const u64_4 lhs, const uint64_t rhs) {
  __m128i r64 = _mm_set1_epi64x(rhs);
  return u64_4(_mm_cmpeq_epi64(lhs.xy, r64), _mm_cmpeq_epi64(lhs.zw, r64)) + u64_4(1);
}

u64_4 operator-(const u64_4 lhs) {
  return u64_4(_mm_sub_epi64(_mm_setzero_si128(), lhs.xy), _mm_sub_epi64(_mm_setzero_si128(), lhs.zw));
}

// (NOT lhs) AND rhs
u64_4 andnot(const u64_4 lhs, const u64_4 rhs) {
  return u64_4(_mm_andnot_si128(lhs.xy, rhs.xy), _mm_andnot_si128(lhs.zw, rhs.zw));
}

u64_4 operator~(const u64_4 lhs) {
  return u64_4(_mm_andnot_si128(lhs.xy, _mm_set1_epi8(0xFF)), _mm_andnot_si128(lhs.zw, _mm_set1_epi8(0xFF)));
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

uint64_t flip(const board &bd, int pos) {
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
  __m128i flipped_xz_yw = _mm_or_si128(flipped.xy, flipped.zw);
  uint64_t lo = _mm_cvtsi128_si64(flipped_xz_yw);
  uint64_t hi = _mm_cvtsi128_si64(_mm_srli_si128(flipped_xz_yw, 8));
  return lo | hi;
}

board put_black_at(const board & bd, int i, int j) {
  uint64_t reverse_bits = flip(bd, i*8 + j);
  return board(
      (bd.black() ^ reverse_bits) |
          UINT64_C(1) << (i * 8 + j),
      bd.white() ^ reverse_bits);
}

board put_black_at_rev(const board & bd, int i, int j) {
  uint64_t reverse_bits = flip(bd, i*8 + j);
  return board(
      bd.white() ^ reverse_bits,
      (bd.black() ^ reverse_bits) |
          UINT64_C(1) << (i * 8 + j));
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

} // namespace state
