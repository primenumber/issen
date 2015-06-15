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

std::string files[4] = {"lsa1m12", "lsa1m14", "lsa1m16", "lsa1m18"};
std::vector<std::vector<double>> vals;
int val_indeces[60] = {
  0,0,0,0,0,0,0,0,0,0,
  0,0,0,1,1,2,2,3,3,3,
  3,3,3,3,3,3,3,3,3,3,
  3,3,3,3,3,3,3,3,3,3,
  3,3,3,3,3,3,3,3,3,3,
  3,3,3,3,3,3,3,3,3,3};

void load16() {
}

void init() {
  vals.resize(4);
  for (int cnt = 0; cnt < 4; ++cnt) {
    std::ifstream ifs(files[cnt]);
    for (int i = 0; i <= subboard::index_max; ++i) {
      double v;
      ifs >> v;
      vals[cnt].push_back(v);
    }
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
  if (bn) {
    if (wn) return bn - wn;
    else return 64;
  } else {
    return -64;
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

int definite_value(const board &bd) {
  board df = bit_manipulations::definites(bd);
  return diff_num(df) * 6;
}

int pos_value(const board &bd) {
  int score = 0;
  const std::array<int, 64> ary = {{
    100, -40,   5,   0,   0,   5, -40, 100,
    -40, -50, -10, -10, -10, -10, -50, -40,
      5, -10,   0,  -3,  -3,   0, -10,   5,
      0, -10,  -3,  -3,  -3,  -3, -10,   0,
      0, -10,  -3,  -3,  -3,  -3, -10,   0,
      5, -10,   0,  -3,  -3,   0, -10,   5,
    -40, -50, -10, -10, -10, -10, -50, -40,
    100, -40,   5,   0,   0,   5, -40, 100
  }};
  for (int i = 0; i < 64; ++i) {
    if (bd.black().get(i)) score += ary[i];
    else if (bd.white().get(i)) score -= ary[i];
  }
  return score;
}

int edge_value_impl(const board &bd) {
  int index = bit_manipulations::toBase3(bd.black(), bd.white());
  switch (index) {
    case 3279: return  200; // .xxxxxx.
    case 6558: return -200; // .oooooo.
    case 3276: return -100; // .xxxxx..
    case 6552: return  100; // .ooooo..
    case 1092: return -100; // ..xxxxx.
    case 2184: return  100; // ..ooooo.
    case 1089: return  100; // ..xxxx..
    case 2178: return -100; // ..oooo..
    default: return 0;
  }
}

int edge_value(const board &bd) {
  using namespace bit_manipulations;
  return edge_value_impl(bd) +
      edge_value_impl(flipVertical(bd)) +
      edge_value_impl(flipDiagA1H8(bd)) +
      edge_value_impl(flipDiagA8H1(bd));
}

int value(const board & bd) {
  //if (state::puttable_black(bd) == 0) return num_value(bd);
  int score = 0;
  score += puttable_value(bd);
  //score += pos_value(bd);
  score += definite_value(bd);
  score += edge_value(bd);
  //score += statistic_value(bd);
  return score;
}

int num_value(const board & bd) {
  return 100 * fixed_diff_num(bd);
}

int statistic_value (const board &bd) {
  int index = val_indeces[64 - bit_manipulations::stone_sum(bd)];
  std::vector<int> indeces = subboard::serialize(bd);
  double res = 0;
  assert(indeces.size() == 46);
  for (int i : indeces) {
    res += vals[index][i];
  }
  return res * 100;
}

} // namespace value