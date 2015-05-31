#include "statistic_value_generator.hpp"

#include <cassert>

#include <algorithm>
#include <iostream>
#include <vector>
#include <tuple>

#include "board.hpp"
#include "bit_manipulations.hpp"
#include "hand.hpp"
#include "utils.hpp"
#include "state.hpp"

namespace sv_gen {

constexpr int pow3(int index) {
  return index ? 3 * pow3(index - 1) : 1;
}

constexpr int sum(const int * ary, int num) {
  int s = 0;
  for (int i = 0; i < num; ++i) {
    s += ary[i];
  }
  return s;
}

constexpr int num_h[4] = {pow3(8), pow3(8), pow3(8), pow3(8)};
constexpr int num_d[8] = {
    0, 0, 0, pow3(4), pow3(5), pow3(6), pow3(7), pow3(8)};
constexpr int num_e = pow3(10);
constexpr int num_c33 = pow3(9);
constexpr int num_c25 = pow3(10);
constexpr int begin_h[4] = {
    1, 1+sum(num_h, 1), 1+sum(num_h, 2), 1+sum(num_h, 3)};
constexpr int begin_d[8] = {
    0, 0, 0, begin_h[3]+num_h[3],
    begin_h[3]+num_h[3]+sum(num_d+3, 1), begin_h[3]+num_h[3]+sum(num_d+3, 2),
    begin_h[3]+num_h[3]+sum(num_d+3, 3), begin_h[3]+num_h[3]+sum(num_d+3, 4)};
//constexpr int begin_e = begin_d[7] + num_d[7];
constexpr int begin_c33 = begin_d[7] + num_d[7];
//constexpr int begin_c25 = begin_c33 + num_c33;

int get_index_horizontal(const board &bd, int index) {
  return bit_manipulations::toBase3(bd.black().lines[index],
      bd.white().lines[index]);
}

int get_index_vertical(const board &bd, int index) {
  return get_index_horizontal(bit_manipulations::flipDiagA1H8(bd), index);
}

int get_index_diagonal_A1H8(const board &bd, int index) {
  board rtbd = bit_manipulations::pseudoRotate45clockwise(bd);
  if (index >= 0) {
    uint8_t black_bit = rtbd.black().lines[index] & (0xFF >> index);
    uint8_t white_bit = rtbd.white().lines[index] & (0xFF >> index);
    return bit_manipulations::toBase3(black_bit, white_bit);
  } else {
    uint8_t black_bit = rtbd.black().lines[8+index] & (0xFF << -index);
    uint8_t white_bit = rtbd.white().lines[8+index] & (0xFF << -index);
    return bit_manipulations::toBase3(black_bit, white_bit);
  }
}

int get_index_diagonal_A8H1(const board &bd, int index) {
  return get_index_diagonal_A1H8(bit_manipulations::flipVertical(bd), index);
}

// *a****b* -> 000000ab
uint8_t get_xbit(uint8_t line) {
  return (uint8_t)((line & 0b01000010) * 17) >> 5;
}

uint16_t get_edge_bits(const bit_board &bbd) {
  uint16_t bit = bbd.lines[0];
  return bit | (uint16_t)get_xbit(bbd.lines[1]) << 8;
}

int get_index_edge(const board &bd) {
  return bit_manipulations::toBase3(get_edge_bits(bd.black()),
      get_edge_bits(bd.white()));
}

uint16_t get_corner_3x3_bits(const bit_board &bbd) {
  uint16_t bit = 0;
  for (int i = 0; i < 3; ++i)
    bit |= (uint16_t)(bbd.lines[i] & 0b111) << (3 * i);
  return bit;
}

int get_index_corner_3x3(const board &bd) {
  return bit_manipulations::toBase3(get_corner_3x3_bits(bd.black()),
      get_corner_3x3_bits(bd.white()));
}

uint16_t get_corner_2x5_bits(const bit_board &bbd) {
  uint16_t bit = 0;
  for (int i = 0; i < 2; ++i)
    bit |= (uint16_t)(bbd.lines[i] & 0b11111) << (5 * i);
  return bit;
}

int get_index_corner_2x5(const board &bd) {
  return bit_manipulations::toBase3(get_corner_2x5_bits(bd.black()),
      get_corner_2x5_bits(bd.white()));
}

std::vector<int> get_cols(const board &bd) {
  std::vector<int> cols;
  cols.push_back(0);
  for (int i = 0; i < 8; ++i) {
    int hi = std::min(i, 7-i);
    cols.push_back(begin_h[hi] + get_index_horizontal(bd, hi));
    cols.push_back(begin_h[hi] + get_index_vertical(bd, hi));
  }
  for (int i = -4; i <= 4; ++i) {
    int di = 7 - std::abs(i);
    cols.push_back(begin_d[di] + get_index_diagonal_A1H8(bd, di));
    cols.push_back(begin_d[di] + get_index_diagonal_A8H1(bd, di));
  }
  for (int i = 0; i < 4; ++i) {
    board tmp = bd;
    if (i & 1) tmp = bit_manipulations::rotate90clockwise(tmp);
    if (i & 2) tmp = bit_manipulations::rotate180(tmp);
    //cols.push_back(begin_e + get_index_edge(tmp));
    cols.push_back(begin_c33 + get_index_corner_3x3(tmp));
  }
  /*
  for (int i = 0; i < 8; ++i) {
    board tmp = bd;
    if (i & 1) tmp = bit_manipulations::rotate90clockwise(tmp);
    if (i & 2) tmp = bit_manipulations::rotate180(tmp);
    if (i & 4) tmp = bit_manipulations::flipVertical(tmp);
    cols.push_back(begin_c25 + get_index_corner_2x5(tmp));
  }
  */
  return cols;
}

int output_one_record(board bd, int turn_number, int row) {
  std::string line;
  int score;
  std::cin >> line >> score;
  for (int i = 0; i < (int)line.size() / 2; ++i) {
    if (64 - bit_manipulations::stone_sum(bd) <= turn_number) break;
    hand h = to_hand(line.substr(i*2, 2));
    if (h != PASS)
      bd = board::reverse_board(state::put_black_at(bd, h/8, h%8));
    else
      bd = board::reverse_board(bd);
  }
  std::vector<int> cols = get_cols(bd);
  std::sort(std::begin(cols), std::end(cols));
  assert(cols.size() == 39);
  for (int col : cols) {
    std::cout << row << ' ' << col << " 1\n";
  }
  return score;
}

void generate_lsprob_input(int turn_number) {
  using std::cin;
  using std::cout;
  using std::endl;
  board bd;
  bool is_black;
  std::tie(bd, is_black) = utils::input_ffo();
  int n;
  cin >> n;
  cout << 39*n << endl;
  std::vector<int> scores(n);
  for (int i = 0; i < n; ++i)
    scores[i] = output_one_record(bd, turn_number, i);
  cout << n << endl;
  for (int i = 0; i < n; ++i) {
    if (i) cout << ' ';
    cout << scores[i];
  }
  cout << endl;
}

} // namespace sv_gen
