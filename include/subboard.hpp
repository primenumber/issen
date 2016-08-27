#pragma once
#include <array>

#include "board.hpp"

namespace subboard {

void init();
extern int index_max;
std::array<int, 46> serialize(const board &bd);

} // namespace subboard
