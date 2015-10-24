#include "generate.hpp"

#include <cstdio>

#include <random>

#include "bit_manipulations.hpp"
#include "state.hpp"
#include "hand.hpp"
#include "utils.hpp"
#include "value.hpp"
#include "tree_manager.hpp"

namespace generator {

static std::mt19937 mt;

void init() {
  std::random_device rd;
  mt.seed(rd());
}

std::vector<board> generate(const board &bd) {
  std::vector<board> ary;
  ary.push_back(bd);
  board tmp = bd;
  bool is_black = true;
  while (!state::is_gameover(tmp)) {
    if (bit_manipulations::stone_sum(tmp) == 54) break;
    auto nexts = state::next_states(tmp);
    std::uniform_int_distribution<> dis(0, nexts.size() - 1);
    tmp = nexts[dis(mt)];
    is_black = !is_black;
    if (is_black)
      ary.push_back(tmp);
    else
      ary.emplace_back(tmp, reverse_construct_t());
  }
  if (!state::is_gameover(tmp)) {
    tree_manager::tree_manager tm(tmp, is_black);
    auto tp = tm.endgame_search();
    for (auto p : std::get<0>(tp)) {
      is_black = !is_black;
      if (is_black)
        ary.push_back(std::get<0>(p));
      else
        ary.emplace_back(std::get<0>(p), reverse_construct_t());
    }
  }
  return ary;
}

void generate_record(const board &bd, size_t n) {
  std::printf("%s", utils::to_s_ffo(bd).c_str());
  std::printf("Black\n");
  std::printf("%zd\n", n);
  for (std::size_t i = 0; i < n; ++i) {
    if ((i % 1000) == 0) fprintf(stderr, "%zd\n", i);
    auto board_record = generate(bd);
    for (std::size_t j = 0; j < board_record.size() - 1; ++j) {
      hand h = hand_from_diff(board_record[j], board_record[j+1]);
      std::printf("%s", to_s(h).c_str());
    }
    std::printf("\n%d\n", value::fixed_diff_num(board_record.back()));
  }
}

} // namespace generator
