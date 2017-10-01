#pragma once
#include <array>

#include "board.hpp"
#include "bit_manipulations.hpp"

namespace subboard {

void init();
extern int index_max;
std::array<int, 46> serialize(const board &bd);

extern uint32_t pow3[13];

template <size_t N>
std::array<uint32_t, 8*N> serialize(const board &bd, const std::array<uint64_t, N> &bits) {
  using namespace bit_manipulations;
  std::array<uint32_t, 8*N> result;
  uint32_t offset = 0;
  std::array<board, 8> transed_bd;
  board tmp = bd;
  for (int i = 0; i < 4; ++i) {
    transed_bd[i] = tmp;
    board tmp2 = flipDiagA8H1(tmp);
    transed_bd[i+4] = tmp2;
    tmp = rotate90clockwise(tmp);
  }
  for (size_t i = 0; i < N; ++i) {
    uint64_t bit = bits[i];
    for (size_t j = 0; j < 8; ++j) {
      uint64_t me_bits = _pext_u64(transed_bd[j].player(), bit);
      uint64_t op_bits = _pext_u64(transed_bd[j].opponent(), bit);
      result[i*8 + j] = offset + toBase3(me_bits, op_bits);
    }
    int count = _popcnt64(bit);
    offset += pow3[count];
  }
  return result;
}

} // namespace subboard
