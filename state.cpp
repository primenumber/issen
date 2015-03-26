#include "state.hpp"

#include <cassert>

#include <iomanip>
#include <iostream>

#include "bit_manipulations.hpp"
#include "line.hpp"

namespace state {

enum class state {
  NONE,
  BLACK,
  WHITE
};

void init() {
  for (int i = 0; i < 6561; ++i) {
    std::array<state, 8> ary;
    int ii = i;
    for (int j = 0; j < 8; ++j) {
      switch (ii % 3) {
       case 1:
        ary[j] = state::BLACK;
        break;
       case 2:
        ary[j] = state::WHITE;
        break;
       default:
        ary[j] = state::NONE;
      }
    }
    uint8_t res = 0;
    /*
    for (int j = 0; j < 8; ++j)
      if (puttable_black_line(ary, j))
        res |= 1 << j;
        */
  }
}


bool puttable_black_at_dir(const board & bd,
    const int i, const int j, const int dir) {
  const int di[8] = {1, 1, 0, -1, -1, -1, 0, 1};
  const int dj[8] = {0, 1, 1, 1, 0, -1, -1, -1};
  for (int k = 1; k < 8; ++k) {
    int ni = i + di[dir] * k;
    int nj = j + dj[dir] * k;
    if (ni < 0 || nj < 0 || ni >= 8 || nj >= 8) return false;
    if (bd.black.get(ni*8+nj)) {
      return k >= 2;
    } else if (!bd.white.get(ni*8+nj)) {
      return false;
    }
  }
  return false;
}

bool puttable_black_at(const board & bd, const int i, const int j) {
  if (bd.black.get(i*8+j) || bd.white.get(i*8+j)) return false;
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
  uint64_t res = 0;
  for (int i = 0; i < 8; ++i)
    res |= (uint64_t)line::puttable_line(bd, i, 0) << (i * 8);
  return res;
}

uint64_t puttable_black_virtical(const board &bd) {
  const board fliped_bd = bit_manipulations::flipDiagA1H8(bd);
  uint64_t res = 0;
  for (int i = 0; i < 8; ++i)
    res |= (uint64_t)line::puttable_line(fliped_bd, i, 0) << (i * 8);
  return bit_manipulations::flipDiagA1H8(res);
}

uint64_t puttable_black_diag_implA8H1(const board &bd) {
  const board prot45_bd = bit_manipulations::pseudoRotate45clockwise(bd);
  uint64_t res = 0;
  for (int i = 0; i < 8; ++i)
    res |= (uint64_t)line::puttable_line(prot45_bd, i, i) << (i * 8);
  return bit_manipulations::pseudoRotate45antiClockwise(res);
}

uint64_t puttable_black_diag_implA1H8(const board &bd) {
  const board prot45a_bd = bit_manipulations::pseudoRotate45antiClockwise(bd);
  uint64_t res = 0;
  for (int i = 0; i < 8; ++i)
    res |= (uint64_t)line::puttable_line(prot45a_bd, i, (8 - i) % 8) << (i * 8);
  return bit_manipulations::pseudoRotate45clockwise(res);
}

uint64_t puttable_black_diag(const board &bd) {
  return bit_manipulations::rotr(
      puttable_black_diag_implA8H1(bd) |
      puttable_black_diag_implA1H8(bd), 8);
}

uint64_t puttable_black(const board &bd) {
  return puttable_black_horizontal(bd) |
      puttable_black_virtical(bd) |
      puttable_black_diag(bd);
}

bool is_gameover(const board &bd) {
  return puttable_black(bd) == 0 &&
      puttable_black(board::reverse_board(bd)) == 0;
}

void put_black_at_dir(board &bd, int i, int j, int dir) {
  const int di[8] = {1, 1, 0, -1, -1, -1, 0, 1};
  const int dj[8] = {0, 1, 1, 1, 0, -1, -1, -1};
  for (int k = 1; k < 8; ++k) {
    int ni = i + di[dir] * k;
    int nj = j + dj[dir] * k;
    if (ni < 0 || nj < 0 || ni >= 8 || nj >= 8) return;
    if (bd.black.get(ni*8+nj)) {
      for (int l = 1; l < k; ++l) {
        int li = i + di[dir] * l;
        int lj = j + dj[dir] * l;
        bd.black.set(li*8+lj);
        bd.white.reset(li*8+lj);
      }
      return;
    } else if (!bd.white.get(ni*8+nj)) {
      return;
    }
  }
}

board put_black_at_naive(const board & bd, int i, int j) {
  board res = bd;
  for (int dir = 0; dir < 8; ++dir)
    put_black_at_dir(res, i, j, dir);
  res.black.set(i*8+j);
  return res;
}

uint64_t put_black_at_horizontal(const board &bd, int i, int j) {
  return ((uint64_t)line::put_line(bd, i, j, 0)) << (i * 8);
}

uint64_t put_black_at_vertical(const board &bd, int i, int j) {
  return bit_manipulations::flipDiagA1H8(
      ((uint64_t)line::put_line(
          bit_manipulations::flipDiagA1H8(bd), j, i, 0)) << (j * 8));
}

uint64_t put_black_at_diag_implA8H1(const board &bd, int i, int j) {
  board prot45_bd = 
      bit_manipulations::pseudoRotate45clockwise(bd);
  return bit_manipulations::pseudoRotate45antiClockwise(
      ((uint64_t)line::put_line(prot45_bd,
          (i + j + 1) % 8, j, (i + j + 1) % 8)) << (((i + j + 1) % 8) * 8));
}

uint64_t put_black_at_diag_implA1H8(const board &bd, int i, int j) {
  board prot45a_bd = 
      bit_manipulations::pseudoRotate45antiClockwise(bd);
  return bit_manipulations::pseudoRotate45clockwise(
      ((uint64_t)line::put_line(prot45a_bd,
          (i + 8 - j) % 8, j, (8 - i + j) % 8)) << (((i + 8 - j) % 8) * 8));
}

uint64_t put_black_at_diag(const board &bd, int i, int j) {
  return bit_manipulations::rotr(
      put_black_at_diag_implA1H8(bd, i, j) |
      put_black_at_diag_implA8H1(bd, i, j), 8);
}

board put_black_at(const board & bd, int i, int j) {
  uint64_t reverse_bits = put_black_at_horizontal(bd, i, j) |
      put_black_at_vertical(bd, i, j) |
      put_black_at_diag(bd, i, j);
  return board(
      bd.black.data ^ reverse_bits |
          UINT64_C(1) << (i * 8 + j),
      bd.white.data ^ reverse_bits);
}

std::vector<board> next_states(const board & bd) {
  std::vector<board> res;
  res.reserve(16);
  bool is_pass = true;
  for (uint64_t bits = puttable_black(bd); bits != 0; bits &= bits - 1) {
    int pos = bit_manipulations::bit_to_pos(bits & -bits);
    res.emplace_back(put_black_at(bd, pos / 8, pos % 8),
        reverse_construct_t());
    is_pass = false;
  }
  if (is_pass) res.emplace_back(bd, reverse_construct_t());
  return res;
}

} // namespace state
