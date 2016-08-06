#include "statistic_value_generator.hpp"

#include <cassert>

#include <algorithm>
#include <iostream>
#include <vector>
#include <tuple>

#include <boost/optional.hpp>

#include "board.hpp"
#include "bit_manipulations.hpp"
#include "hand.hpp"
#include "utils.hpp"
#include "state.hpp"
#include "subboard.hpp"
#include "record.hpp"
#include "value.hpp"

namespace sv_gen {

struct record_and_indeces {
  record::game_record record;
  std::vector<int> indeces;
  int puttable;
  int puttable_op;
};

boost::optional<record_and_indeces> get_one_record(board bd, int turn_number) {
  std::string line;
  int score;
  std::cin >> line >> score;
  bool is_valid = false;
  board snap;
  record::game_record gr;
  bool is_black = true;
  for (int i = 0; i < (int)line.size() / 2; ++i) {
    hand h = to_hand(line.substr(i*2, 2));
    gr.hands.push_back(h);
    if (64 - bit_manipulations::stone_sum(bd) == turn_number && h != PASS) {
      is_valid = true;
      snap = bd;
      is_black = i % 2 == 0;
    }
    if (h != PASS)
      bd = state::put_black_at_rev(bd, h/8, h%8);
    else
      bd = board::reverse_board(bd);
  }
  gr.result = is_black ? score : -score;
  if (!is_valid) return boost::none;
  return boost::optional<record_and_indeces>(record_and_indeces{gr,
      subboard::serialize(snap),
      value::puttable_black_count(snap),
      value::puttable_black_count(board::reverse_board(snap))});
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
  std::cerr<<n<<endl;
  std::vector<record_and_indeces> vri;
  for (int i = 0; i < n; ++i) {
    boost::optional<record_and_indeces> ri_opt =
        get_one_record(bd, turn_number);
    if (ri_opt) vri.push_back(*ri_opt);
  }
  n = vri.size();
  cout << 49*n << endl;
  for (int i = 0; i < n; ++i) {
    std::vector<int> s = vri[i].indeces;
    std::sort(std::begin(s), std::end(s));
    for (int j = 0; j < 46; ++j) {
      std::cout << i << ' ' << s[j] << ' ' << 1 << '\n';
    }
    int offset = subboard::index_max;
    std::cout << i << ' ' << offset << ' ' << vri[i].puttable << '\n';
    std::cout << i << ' ' << (offset+1) << ' ' << vri[i].puttable_op << '\n';
    std::cout << i << ' ' << (offset+2) << ' ' << 1 << '\n';
  }
  cout << n << endl;
  for (int i = 0; i < n; ++i) {
    if (i) cout << ' ';
    cout << vri[i].record.result;
  }
  cout << endl;
}

} // namespace sv_gen
