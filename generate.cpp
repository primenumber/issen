#include "generate.hpp"

#include <random>

#include "state.hpp"

namespace generator {

static std::mt19937 mt;

void init() {
  std::random_device rd;
  mt.seed(rd());
}

std::vector<board> genarate(const board &bd) {
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

} // namespace generator
