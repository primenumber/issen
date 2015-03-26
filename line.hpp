#pragma once

#include "board.hpp"

namespace line {

void init();

bool is_puttable_line(const board &, int line, int pos, int separator);
bool is_puttable_line(uint8_t black, uint8_t white, int pos, int separator);
uint8_t puttable_line(const board &, int line, int separator);
uint8_t puttable_line(uint8_t black, uint8_t white, int separator);
uint8_t put_line(const board &, int line, int pos, int separator);
uint8_t put_line(uint8_t black, uint8_t white, int pos, int separator);

} // namespace line
