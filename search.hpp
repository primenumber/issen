#pragma once
#include <algorithm>
#include "board.hpp"
#include "state.hpp"

namespace search {

template <typename ValueFunc>
int negamax(const board & bd, const int depth, ValueFunc func) {
  if (depth == 0) {
    return func(bd);
  } else {
    int maxv = -10000;
    for (auto nx : state::next_states(bd))
      maxv = std::max(maxv, -negamax(nx, depth - 1, func));
    return maxv;
  }
}

template <typename ValueFunc>
int negaalpha(const board & bd, const int depth, ValueFunc func,
    int alpha, int beta) {
  if (depth <= 0 || state::is_gameover(bd)) {
    return func(bd);
  } else if (state::puttable_black(bd) == 0) {
    return -negaalpha(board(bd, reverse_construct_t()), depth - 1, func, -beta, -alpha);
  } else {
    for (auto nx : state::next_states(bd)) {
      int val = -negaalpha(nx, depth - 1, func, -beta, -alpha);
      if (beta <= val) return beta;
      alpha = std::max(alpha, val);
    }
    return alpha;
  }
} 

template <typename ValueFunc>
board get_opt(const board & bd, const int depth, ValueFunc func) {
  board res;
  int maxv = -10000;
  for (auto nx : state::next_states(bd)) {
    int val = -negaalpha(nx, depth - 1, func, -10000, 10000);
    if (val > maxv) {
      maxv = val;
      res = nx;
    }
  }
  return res;
}

} // namespace search
