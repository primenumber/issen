#include "value.hpp"

namespace value {

void init() {
}

int puttable_value(const board &bd) {
  return 10 * (_popcnt64(state::puttable_black(bd)) -
      _popcnt64(state::puttable_black(board::reverse_board(bd))));
}

int pos_value(const board &bd) {
  const std::array<int, 64> ary = {{
    100, -50,  15,  15,  15,  15, -50, 100,
    -50, -75, -10, -10, -10, -10, -75, -50,
     15, -10,   0,  -1,  -1,   0, -10,  15,
     15, -10,  -1,  -1,  -1,  -1, -10,  15,
     15, -10,  -1,  -1,  -1,  -1, -10,  15,
     15, -10,   0,  -1,  -1,   0, -10,  15,
    -50, -75, -10, -10, -10, -10, -75, -50,
    100, -50,  15,  15,  15,  15, -50, 100
  }};
  int score = 0;
  for (int i = 0; i < 64; ++i) {
    if (bd.black.get(i)) score += ary[i];
    else if (bd.white.get(i)) score -= ary[i];
  }
  return score;
}

int value(const board & bd) {
  if (state::puttable_black(bd) == 0) return num_value(bd);
  int score = 0;
  score += puttable_value(bd);
  score += pos_value(bd);
  return score;
}

int num_value(const board & bd) {
  return 100 * (_popcnt64(bd.black.data) - _popcnt64(bd.white.data));
}

} // namespace value
