#pragma once
#include "board.hpp"

namespace value {

constexpr int VALUE_MAX = 6400;

void init();
int value(const board &);
int diff_num(const board &);
int fixed_diff_num(const board &);
int puttable_value(const board &);
int num_value(const board &);
int statistic_value(const board &);

} // namespace value
