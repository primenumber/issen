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

namespace sv_gen {

using record_and_indeces = std::tuple<record::game_record, std::vector<int>>;

boost::optional<record_and_indeces> get_one_record(board bd, int turn_number) {
  std::string line;
  int score;
  std::cin >> line >> score;
  bool is_valid = false;
  board snap;
  record::game_record gr;
  gr.result = score;
  for (int i = 0; i < (int)line.size() / 2; ++i) {
    if (64 - bit_manipulations::stone_sum(bd) == turn_number && !is_valid) {
      is_valid = true;
      snap = bd;
    }
    hand h = to_hand(line.substr(i*2, 2));
    gr.hands.push_back(h);
    if (h != PASS)
      bd = state::put_black_at_rev(bd, h/8, h%8);
    else
      bd = board::reverse_board(bd);
  }
  if (!is_valid) return boost::none;
  return boost::optional<record_and_indeces>(std::make_tuple(gr,
      subboard::serialize(snap)));
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
  cout << 46*n << endl;
  for (int i = 0; i < n; ++i) {
    std::vector<int> s = std::get<1>(vri[i]);
    std::sort(std::begin(s), std::end(s));
    for (int j = 0; j < 46; ++j) {
      std::cout << i << ' ' << s[j] << ' ' << 1 << std::endl;
    }
  }
  cout << n << endl;
  for (int i = 0; i < n; ++i) {
    if (i) cout << ' ';
    cout << std::get<0>(vri[i]).result;
  }
  cout << endl;
}

} // namespace sv_gen
