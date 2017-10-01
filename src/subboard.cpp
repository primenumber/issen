#include "subboard.hpp"

#include <algorithm>
#include <vector>

#include "bit_manipulations.hpp"

namespace subboard {

uint32_t pow3[13];

void init() {
  pow3[0] = 1;
  for (int i = 0; i < 12; ++i) {
    pow3[i+1] = pow3[i] * 3;
  }
}

} // namespace subboard
