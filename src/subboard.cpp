#include "subboard.hpp"

#include <algorithm>
#include <vector>

#include "bit_manipulations.hpp"

namespace subboard {

int index_max;

constexpr std::array<int, 11> stones = {
  8, 8, 8, // horizontal
  8, 7, 6, 5, 4, // diagonal
  10, // edge
  9, 10 // corner
};

std::vector<int> index_begin;

constexpr int pow3(int index) {
  return index ? 3 * pow3(index - 1) : 1;
}

void init() {
  index_begin.resize(stones.size());
  index_begin[0] = 0;
  for (int i = 1; i < (int)stones.size(); ++i) {
    index_begin[i] = index_begin[i-1] + pow3(stones[i-1]);
  }
  index_max = index_begin.back() + pow3(stones.back()) - 1;
}

uint16_t rev_bit(uint16_t bits, int size) {
  switch (size) {
    case 4:
      bits = ((bits & 0x3) << 2) | ((bits & 0xC) >> 2);
      return ((bits & 0x5) << 1) | ((bits & 0xA) >> 1);
    case 5:
      bits = ((bits & 0x3) << 3) | ((bits & 0x18) >> 3) | (bits & 0x4);
      return ((bits & 0x9) << 1) | ((bits & 0x12) >> 1) | (bits & 0x4);
    case 6:
      bits = ((bits & 0x7) << 3) | ((bits & 0x38) >> 3);
      return ((bits & 0x15) << 1) | ((bits & 0x2A) >> 1) | (bits & 0x12);
    case 7:
      bits = ((bits & 0x7) << 4) | ((bits & 0x70) >> 4) | (bits & 0x8);
      return ((bits & 0x11) << 2) | ((bits & 0x44) >> 2) | (bits & 0x2A);
    case 8:
      bits = ((bits & 0x0F) << 4) | ((bits & 0xF0) >> 4);
      bits = ((bits & 0x33) << 2) | ((bits & 0xCC) >> 2);
      return ((bits & 0x55) << 1) | ((bits & 0xAA) >> 1);
    case 9:
      bits = ((bits & 0x0F) << 5) | ((bits & 0x1E0) >> 5) | (bits & 0x10);
      bits = ((bits & 0x63) << 2) | ((bits & 0x18C) >> 2) | (bits & 0x10);
      return ((bits & 0xA5) << 1) | ((bits & 0x14A) >> 1) | (bits & 0x10);
    case 10:
      bits = ((bits & 0x1F) << 5) | ((bits & 0x3E0) >> 5);
      bits = ((bits & 0x63) << 3) | ((bits & 0x318) >> 3) | (bits & 0x84);
      return ((bits & 0x129) << 1) | ((bits & 0x252) >> 1) | (bits & 0x84);
    default: return bits;
  }
}

uint16_t rev_bit_diag(uint16_t bits) {
  return (bits & 0x111) |
    ((bits & 0x88) >> 2) | ((bits & 0x40) >> 4) |
    ((bits & 0x22) << 2) | ((bits & 0x04) << 4);
}

int to_index(uint64_t black, uint64_t white, int size) {
  return std::min(bit_manipulations::toBase3(black, white),
      bit_manipulations::toBase3(rev_bit(black, size), rev_bit(white, size)));
}

int to_index_diag(uint64_t black, uint64_t white) {
  return std::min(bit_manipulations::toBase3(black, white),
      bit_manipulations::toBase3(rev_bit_diag(black), rev_bit_diag(white)));
}

int to_index_asymmetry(uint64_t black, uint64_t white) {
  return bit_manipulations::toBase3(black, white);
}

int get_index_horizontal(const board &bd, int index) {
  int hi = std::min(index, 7-index);
  return index_begin[hi-1] +
      to_index(0xFF & (bd.black() >> (index*8)),
          0xFF & (bd.white() >> (index*8)), 8);
}

int get_index_diagonal(const board &rtbd, int index) {
  uint8_t black_bit, white_bit;
  if (index >= 0) {
    black_bit = (rtbd.black() >> (index*9)) & (0xFF >> index);
    white_bit = (rtbd.white() >> (index*9)) & (0xFF >> index);
  } else {
    black_bit = (rtbd.black() >> ((8+index)*8)) & (0xFF >> -index);
    white_bit = (rtbd.white() >> ((8+index)*8)) & (0xFF >> -index);
  }
  return index_begin[3 + std::abs(index)] +
      to_index(black_bit, white_bit, 8-std::abs(index));
}

// *a****b* -> 000000ab
uint16_t get_xbit(uint8_t line) {
  return (((line & 0b01000010) * 17) & 0b10000000010) >> 1;
}

uint16_t get_edge_bits(const half_board &bbd) {
  uint16_t bit = bbd & 0xFF;
  return bit << 1 | get_xbit(bbd >> 8);
}

int get_index_edge(const board &bd) {
  return index_begin[8] +
      to_index(get_edge_bits(bd.black()), get_edge_bits(bd.white()), 10);
}

int get_index_corner_3x3(const board &bd) {
  return index_begin[9] + to_index_diag(
      _pext_u64(bd.black(), 0x070707),
      _pext_u64(bd.white(), 0x070707));
}

int get_index_corner_2x5(const board &bd) {
  return index_begin[10] + to_index_asymmetry(
      _pext_u64(bd.black(), 0x1F1F),
      _pext_u64(bd.white(), 0x1F1F));
}

std::array<int, 46> serialize(const board &bd) {
  std::array<int, 46> cols;
  int cnt = 0;
  board fdbd = bit_manipulations::flipDiagA1H8(bd);
  for (int i = 1; i < 7; ++i) {
    cols[cnt++] = get_index_horizontal(bd, i);
    cols[cnt++] = get_index_horizontal(fdbd, i);
  }
  board rtbd = bit_manipulations::pseudoRotate45clockwise(bd);
  board rtfbd = bit_manipulations::pseudoRotate45clockwise(bit_manipulations::flipVertical(bd));
  for (int i = -4; i <= 4; ++i) {
    cols[cnt++] = get_index_diagonal(rtbd, i);
    cols[cnt++] = get_index_diagonal(rtfbd, i);
  }
  for (int i = 0; i < 8; ++i) {
    board tmp = bd;
    if (i & 1) tmp = bit_manipulations::rotate90clockwise(tmp);
    if (i & 2) tmp = bit_manipulations::rotate180(tmp);
    if (i & 4) tmp = bit_manipulations::flipVertical(tmp);
    cols[cnt++] = get_index_corner_2x5(tmp);
    if (i < 4) {
      cols[cnt++] = get_index_edge(tmp);
      cols[cnt++] = get_index_corner_3x3(tmp);
    }
  }
  return cols;
}

} // namespace subboard
