#include "value.hpp"

namespace value {

void init() {
}

int value(const board & bd) {
  if (state::puttable_black(bd) == 0) return num_value(bd);
  std::array<int, 64> ary = {{
    100, -50,  15,  15,  15,  15, -50, 100,
    -50, -75, -10, -10, -10, -10, -75, -50,
     15, -10,   0,  -5,  -5,   0, -10,  15,
     15, -10,  -5,  -5,  -5,  -5, -10,  15,
     15, -10,  -5,  -5,  -5,  -5, -10,  15,
     15, -10,   0,  -5,  -5,   0, -10,  15,
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

int num_value(const board & bd) {
  return 100 * (_popcnt64(bd.black.data) - _popcnt64(bd.white.data));
}

} // namespace value
