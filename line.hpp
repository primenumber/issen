#pragma once

#include "board.hpp"

bool is_puttable_line(const board &, int, int, int);
uint8_t puttable_line(const board &, int, int);
