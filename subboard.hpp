#pragma once
#include <vector>

#include "board.hpp"

namespace subboard {

void init();
extern int index_max;
std::vector<int> serialize(const board &bd);

} // namespace subboard
