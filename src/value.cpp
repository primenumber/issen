#include "value.hpp"

#include <cassert>

#include <iostream>
#include <fstream>
#include <string>

#include "bit_manipulations.hpp"
#include "state.hpp"
#include "utils.hpp"
#include "subboard.hpp"

namespace value {

std::string files[] = {
  "lsval/lsval4", "lsval/lsval5", "lsval/lsval6", "lsval/lsval7",
  "lsval/lsval8", "lsval/lsval9", "lsval/lsval10", "lsval/lsval11",
  "lsval/lsval12", "lsval/lsval13", "lsval/lsval14", "lsval/lsval15",
  "lsval/lsval16"};
std::vector<std::vector<float>> vals;
std::vector<float> puttable_coeff;
std::vector<float> puttable_op_coeff;
std::vector<float> const_offset;

int val_indeces[60] = {
   0, 0, 0, 0, 0, 1, 2, 3, 4, 5,
   6, 7, 8, 9,10,11,12,12,12,12,
  12,12,12,12,12,12,12,12,12,12,
  12,12,12,12,12,12,12,12,12,12,
  12,12,12,12,12,12,12,12,12,12,
  12,12,12,12,12,12,12,12,12,12};

void load16() {
}

void init() {
  int n = 13;
  vals.resize(n);
  const_offset.resize(n);
  puttable_coeff.resize(n);
  puttable_op_coeff.resize(n);
  for (int cnt = 0; cnt < n; ++cnt) {
    std::ifstream ifs(files[cnt]);
    for (int i = 0; i <= subboard::index_max; ++i) {
      float v = 0;
      ifs >> v;
      vals[cnt].push_back(v);
    }
    ifs >> puttable_coeff[cnt] >> puttable_op_coeff[cnt] >> const_offset[cnt];
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
  std::vector<int> indeces = subboard::serialize(bd);
  float res = const_offset[index];
  assert(indeces.size() == 46);
  for (int i : indeces) {
    res += vals[index][i];
  }
  res += puttable_black_count(bd) * puttable_coeff[index];
  res += puttable_black_count(board::reverse_board(bd)) * puttable_op_coeff[index];
  return std::max(-6400, std::min(6400, int(res * 100)));
}

} // namespace value
