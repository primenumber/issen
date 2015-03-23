#include "line.hpp"

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

uint8_t puttable_line(const board &bd, int line, int separator) {
  uint8_t res = 0;
  for (int i = 0; i < 8; ++i)
    if (is_puttable_line(bd, line, i, separator))
      res |= 1 << i;
  return res;
}
