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
#include "subboard.hpp"

namespace sv_gen {

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
  std::vector<int> cols = subboard::serialize(bd);
  std::sort(std::begin(cols), std::end(cols));
  assert(cols.size() == 46);
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
  cout << 46*n << endl;
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
