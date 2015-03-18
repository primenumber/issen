#pragma once
#include <algorithm>
#include "board.hpp"

template <typename ValueFunc>
int negamax(const board & bd, const int depth, ValueFunc func) {
  if (depth == 0) {
    return func(bd);
  } else {
    int maxv = -10000;
    for (auto nx : next_states(bd))
      maxv = std::max(maxv, -negamax(nx, depth - 1, func));
    return maxv;
  }
}

template <typename ValueFunc>
int negaalpha(const board & bd, const int depth, ValueFunc func,
    int alpha, int beta) {
  if (depth == 0) {
    return func(bd);
  } else {
    for (auto nx : next_states(bd)) {
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
  for (auto nx : next_states(bd)) {
    int val = -negaalpha(nx, depth - 1, func, -10000, 10000);
    if (val > maxv) {
      maxv = val;
      res = nx;
    }
  }
  return res;
}