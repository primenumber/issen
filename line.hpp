#pragma once

#include "board.hpp"

namespace line {

bool is_puttable_line(const board &, int, int, int);
uint8_t puttable_line(const board &, int, int);

} // namespace line
