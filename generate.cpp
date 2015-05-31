#include "generate.hpp"

#include <cstdio>

#include <random>

#include "state.hpp"
#include "hand.hpp"
#include "utils.hpp"
#include "value.hpp"

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
  while (!state::is_gameover(tmp)) {
    auto nexts = state::next_states(tmp);
    std::uniform_int_distribution<> dis(0, nexts.size() - 1);
    tmp = nexts[dis(mt)];
    ary.push_back(tmp);
  }
  return ary;
}

void generate_record(const board &bd, size_t n) {
  std::printf("%s", utils::to_s_ffo(bd).c_str());
  std::printf("Black\n");
  std::printf("%zd\n", n);
  for (int i = 0; i < n; ++i) {
    auto board_record = generate(bd);
    for (int j = 0; j < board_record.size() - 1; ++j) {
      hand h = hand_from_diff(board_record[j], board_record[j+1]);
      std::printf("%s", to_s(h).c_str());
    }
    std::printf("\n%d\n", value::fixed_diff_num(board_record.back()));
  }
}

} // namespace generator
