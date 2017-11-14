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
  "lsval60_b_2", "lsval59_b_2", "lsval58_b_2", "lsval57_b_2",
  "lsval56_b_2", "lsval55_b_2", "lsval54_b_2", "lsval53_b_2",
  "lsval52_b_2", "lsval51_b_2", "lsval50_b_2", "lsval49_b_2",
  "lsval48_b_2", "lsval47_b_2", "lsval46_b_2", "lsval45_b_2",
  "lsval44_b_2", "lsval43_b_2", "lsval42_b_2", "lsval41_b_2",
  "lsval40_b_2", "lsval39_b_2", "lsval38_b_2", "lsval37_b_2",
  "lsval36_b_2", "lsval35_b_2", "lsval34_b_2", "lsval33_b_2",
  "lsval32_b_2", "lsval31_b_2", "lsval30_b_2", "lsval29_b_2",
  "lsval28_b_2", "lsval27_b_2", "lsval26_b_2", "lsval25_b_2",
  "lsval24_b_2", "lsval23_b_2", "lsval22_b_2", "lsval21_b_2",
  "lsval20_b_2", "lsval19_b_2", "lsval18_b_2", "lsval17_b_2",
  "lsval16_b_2", "lsval15_b_2", "lsval14_b_2", "lsval13_b_2"};

std::vector<std::vector<double>> vals;
std::vector<double> puttable_coeff;
std::vector<double> puttable_op_coeff;
std::vector<double> const_offset;

int val_indeces[61] = {
   0,  0,  0,  0,  0,  1,  2,  3,  4,  5,
   6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
  16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
  26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
  36, 37, 38, 39, 40, 41, 42, 43, 44, 45,
  46, 47, 47, 47, 47, 47, 47, 47, 47, 47,
  47};

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
    if (!ifs) {
      std::cerr << "cannot open file: " << val_pos << std::endl;
      continue;
    }
    vals[cnt].resize(offset_all);
    ifs.read((char*)vals[cnt].data(), sizeof(double) * offset_all);
    for (size_t i = 0; i < offset_all; ++i) {
      vals[cnt][i] *= 100;
    }
    double co;
    ifs.read((char*)&co, sizeof(double));
    const_offset[cnt] = co * 100;
  }
}

int diff_num(const board &bd) {
  int bn = _popcnt64(bd.player());
  int wn = _popcnt64(bd.opponent());
  return bn - wn;
}

int fixed_diff_num(const board &bd) {
  int bn = _popcnt64(bd.player());
  int wn = _popcnt64(bd.opponent());
  if (bn > wn) {
    return 64 - 2*wn;
  } else if (wn > bn) {
    return -(64 - 2*bn);
  } else {
    return 0;
  }
}

int puttable_value(const board &bd) {
  int b = state::mobility_count(bd);
  int w = state::mobility_count(board::reverse_board(bd));
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
  int b = state::mobility_count(bd);
  int w = state::mobility_count(board::reverse_board(bd));
  return b - w;
}

int value(const board & bd) {
  return statistic_value(bd);
}

int num_value(const board & bd) {
  return 100 * fixed_diff_num(bd);
}

int statistic_value_impl(const board bd) {
  int index = val_indeces[64 - bit_manipulations::stone_sum(bd)];
  auto indeces = subboard::serialize(bd, bits);
  int res = const_offset[index];
  for (int i : indeces) {
    res += vals[index][i];
  }
  return std::max(-6399, std::min(6399, res));
}

int statistic_value (const board &bd) {
  int pcnt = state::mobility_count(bd);
  if (pcnt == 0) {
    int ocnt = state::mobility_count(board::reverse_board(bd));
    if (ocnt == 0) {
      return num_value(bd);
    } else {
      return -statistic_value_impl(board::reverse_board(bd));
    }
  } else {
    return statistic_value_impl(bd);
  }
}

} // namespace value
