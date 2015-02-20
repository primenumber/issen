#pragma once
#include <vector>

#include "board.hpp"

uint64_t puttable_black(const board &);
board put_black_at(const board &, int, int);
std::vector<board> next_states(const board &);
