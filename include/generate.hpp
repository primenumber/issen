#pragma once
#include <vector>

#include "board.hpp"

namespace generator {

void init();
std::vector<board> generate(const board &);
void generate_record(const board &, size_t);

} // namespace generator
