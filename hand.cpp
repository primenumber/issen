#include "hand.hpp"

hand hand_from_diff(const board &old_b, const board &new_b) {
  uint64_t old_bits = old_b.black | old_b.white;
  uint64_t new_bits = new_b.black | new_b.white;
  for (hand i = 0; i < 64; ++i)
    if (((old_bits >> i) & 1) == 0 && ((new_bits >> i) & 1) == 1)
      return i;
  return PASS;
}
