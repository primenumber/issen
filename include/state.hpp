#pragma once
#include <vector>

#include "board.hpp"

namespace state {

void init();

uint64_t puttable_black(const board &);
bool is_gameover(const board &);
board put_black_at(const board &, int);
board put_black_at(const board &, int, int);
board put_black_at_rev(const board &, int);
board put_black_at_rev(const board &, int, int);
std::vector<board> next_states(const board &);
bool next_states(const board &, std::vector<board> &res);
std::vector<board> next_states(const board &, uint64_t bits);
bool next_states(const board &, uint64_t bits, std::vector<board> &res);

} // namespace state
