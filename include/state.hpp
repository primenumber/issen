#pragma once
#include <vector>

#include "board.hpp"
#include "bit_manipulations.hpp"

namespace state {

uint64_t puttable_black(const board &);
bool is_gameover(const board &);
board put_black_at(const board &, int);
board put_black_at(const board &, int, int);
board put_black_at_rev(const board &, int);
board put_black_at_rev(const board &, int, int);
std::vector<board> next_states(const board &);
bool next_states(const board &, std::vector<board> &res);

template <size_t N>
int next_states(const board & bd, std::array<board, N> &res) {
  int index = 0;
  bool is_pass = true;
  for (uint64_t bits = ~bit_manipulations::stones(bd); bits; bits = _blsr_u64(bits)) {
    int pos = bit_manipulations::bit_to_pos(_blsi_u64(bits));
    board next = put_black_at_rev(bd, pos);
    if (next.black() != bd.white()) {
      res[index++] = next;
      is_pass = false;
    }
  }
  if (is_pass) res[index++] = board::reverse_board(bd);
  return is_pass ? 0 : index;
}
std::vector<board> next_states(const board &, uint64_t bits);
bool next_states(const board &, uint64_t bits, std::vector<board> &res);

} // namespace state
