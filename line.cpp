#include "line.hpp"

#include <cassert>

#include <array>
#include <vector>

#include "bit_manipulations.hpp"

namespace line {

std::array<std::array<uint8_t, 6561>, 8> puttable_line_table;

uint8_t puttable_line_naive(const board &, int, int);

void init() {
  for (int i = 0; i < 256; ++i) {
    for (int j = 0; j < 256; ++j) {
      if (i & j) continue;
      board bd((uint64_t)i, (uint64_t)j);
      int index = bit_manipulations::toBase3(i, j);
      for (int k = 0; k < 8; ++k) {
        puttable_line_table[k][index] = puttable_line_naive(bd, 0, k);
      }
    }
  }
}

bool is_puttable_line_forward(const board &bd, int line8,
    int pos, int separator) {
  int limit = (pos >= separator) ? 8 : separator;
  for (int i = pos + 1; i < limit; ++i) {
    if (bd.black.get(line8 + i)) {
      return (i > pos + 1);
    } else if (!bd.white.get(line8 + i))
      return false;
  }
  return false;
}

bool is_puttable_line_backward(const board &bd, int line8,
    int pos, int separator) {
  int limit = (pos >= separator) ? separator : 0;
  for (int i = pos - 1; i >= limit; --i) {
    if (bd.black.get(line8 + i)) {
      return (i < pos - 1);
    } else if (!bd.white.get(line8 + i))
      return false;
  }
  return false;
}

bool is_puttable_line(const board &bd, int line,
    int pos, int separator) {
  line *= 8;
  if (bd.black.get(line + pos) || bd.white.get(line + pos))
    return false;
  return is_puttable_line_forward(bd, line, pos, separator) ||
      is_puttable_line_backward(bd, line, pos, separator);
}

uint8_t puttable_line_naive(const board &bd, int line, int separator) {
  uint8_t res = 0;
  for (int i = 0; i < 8; ++i)
    if (is_puttable_line(bd, line, i, separator))
      res |= 1 << i;
  return res;
}

uint8_t puttable_line(const board &bd, int line, int separator) {
  uint8_t bb = bd.black.data >> (line * 8);
  uint8_t wb = bd.white.data >> (line * 8);
  return puttable_line_table[separator][bit_manipulations::toBase3(bb, wb)];
}

} // namespace line;
