#include "value.hpp"

#include <cassert>
#include <cmath>
#include <cstdlib>

#include <iostream>
#include <fstream>
#include <string>

#include "bit_manipulations.hpp"
#include "state.hpp"
#include "utils.hpp"
#include "subboard.hpp"

namespace value {

std::vector<std::string> files = {
  "lsval60", "lsval59", "lsval58", "lsval57",
  "lsval56", "lsval55", "lsval54", "lsval53",
  "lsval52", "lsval51", "lsval50", "lsval49",
  "lsval48", "lsval47", "lsval46"};
std::vector<std::vector<int16_t>> vals;
std::vector<int16_t> puttable_coeff;
std::vector<int16_t> puttable_op_coeff;
std::vector<int16_t> const_offset;

int val_indeces[60] = {
   0,  0,  0,  0,  0,  1,  2,  3,  4,  5,
   6,  7,  8,  9, 10, 11, 12, 13, 13, 13,
  13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
  13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
  13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
  13, 13, 13, 13, 13, 13, 13, 13, 13, 13};

void load16() {
}

constexpr int patterns = 13;

std::array<uint64_t, patterns> bits;
std::array<uint32_t, patterns+1> offset;

void init() {
  int n = files.size();
  vals.resize(n);
  const_offset.resize(n);
  puttable_coeff.resize(n);
  puttable_op_coeff.resize(n);
  char *val_dir = std::getenv("VAL_PATH");
  std::ifstream sb_ifs("subboard.txt");
  std::string line;
  uint32_t offset_all = 0;
  int pow3[13];
  pow3[0] = 1;
  for (int i = 0; i < 12; ++i) pow3[i+1] = 3 * pow3[i];
  for (int i = 0; i < patterns; ++i) {
    uint64_t bit = 0;
    for (int j = 0; j < 8; ++j) {
      std::getline(sb_ifs, line);
      for (int k = 0; k < 8; ++k) {
        if (line[k] == 'o') {
          bit |= UINT64_C(1) << (j*8 + k);
        }
      }
    }
    bits[i] = bit;
    offset[i] = offset_all;
    offset_all += pow3[_popcnt64(bit)];
    if (i != patterns - 1) std::getline(sb_ifs, line);
  }
  offset[patterns] = offset_all;
  for (int cnt = 0; cnt < n; ++cnt) {
    std::string val_pos = val_dir == nullptr ? files[cnt] : val_dir + ("/" + files[cnt]);
    std::ifstream ifs(val_pos);
    if (!ifs) std::cerr << "cannot open file: " << val_pos << std::endl;
    for (int i = 0; i < offset_all; ++i) {
      double v = 0;
      ifs >> v;
      vals[cnt].push_back(round(v * 100));
    }
    double pc, poc, co;
    ifs >> pc >> poc >> co;
    puttable_coeff[cnt] = std::round(pc * 100);
    puttable_op_coeff[cnt] = std::round(poc * 100);
    const_offset[cnt] = std::round(co * 100);
  }
}

int diff_num(const board &bd) {
  int bn = _popcnt64(bd.black());
  int wn = _popcnt64(bd.white());
  return bn - wn;
}

int fixed_diff_num(const board &bd) {
  int bn = _popcnt64(bd.black());
  int wn = _popcnt64(bd.white());
  if (bn > wn) {
    return 64 - 2*wn;
  } else if (wn > bn) {
    return -(64 - 2*bn);
  } else {
    return 0;
  }
}

int puttable_value(const board &bd) {
  int b = _popcnt64(state::puttable_black(bd));
  int w = _popcnt64(state::puttable_black(board::reverse_board(bd)));
  if (b) {
    if (w) {
      return 10 * (b - w);
    } else {
      return 10 * 64;
    }
  } else {
    return -10 * 64;
  }
}

int puttable_diff(const board &bd) {
  int b = _popcnt64(state::puttable_black(bd));
  int w = _popcnt64(state::puttable_black(board::reverse_board(bd)));
  return b - w;
}

int puttable_black_count(const board &bd) {
  return _popcnt64(state::puttable_black(bd));
}

int value(const board & bd) {
  return statistic_value(bd);
}

int num_value(const board & bd) {
  return 100 * fixed_diff_num(bd);
}

int statistic_value (const board &bd) {
  int index = val_indeces[64 - bit_manipulations::stone_sum(bd)];
  auto indeces = subboard::serialize(bd, bits);
  int res = const_offset[index];
  for (int i : indeces) {
    res += vals[index][i];
  }
  res += puttable_black_count(bd) * puttable_coeff[index];
  res += puttable_black_count(board::reverse_board(bd)) * puttable_op_coeff[index];
  return std::max(-6400, std::min(6400, res));
}

} // namespace value
