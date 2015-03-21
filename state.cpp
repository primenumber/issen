#include "state.hpp"

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

uint64_t puttable_black(const board & bd) {
  uint64_t res = 0;
  for (int i = 0; i < 8; ++i)
    for (int j = 0; j < 8; ++j)
      if (puttable_black_at(bd, i, j))
        res |= ((uint64_t)1 << (i * 8 + j));
  return res;
}

bool is_gameover(const board &bd) {
  return puttable_black(bd) == 0 &&
      puttable_black(board(bd, reverse_construct_t())) == 0;
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

board put_black_at(const board & bd, int i, int j) {
  board res = bd;
  for (int dir = 0; dir < 8; ++dir)
    put_black_at_dir(res, i, j, dir);
  res.black.set(i*8+j);
  return res;
}

std::vector<board> next_states(const board & bd) {
  std::vector<board> res;
  bool is_pass = true;
  for (int i = 0; i < 8; ++i)
    for (int j = 0; j < 8; ++j)
      if (puttable_black_at(bd, i, j)) {
        res.emplace_back(put_black_at(bd, i, j),
            reverse_construct_t());
        is_pass = false;
      }
  if (is_pass) res.emplace_back(bd, reverse_construct_t());
  return res;
}

} // namespace state
