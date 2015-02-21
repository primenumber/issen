#pragma once
#include "board.hpp"

template <typename ValueFunc>
int negamax(const board & bd, int depth, ValueFunc func) {
  if (depth) {
    return func(bd);
  } else {
    int maxv = -10000;
    for (auto nx : next_states(bd))
      maxv = max(maxv, -negamax(nx, depth - 1, func));
    return maxv;
  }
}
