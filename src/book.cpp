#include "book.hpp"
#include <fstream>
#include "value.hpp"
#include "state.hpp"

namespace book {

Book book;

void init() {
  int n;
  std::ifstream ifs("small.book");
  ifs >> n;
  for (int i = 0; i < n; ++i) {
    std::string b81;
    int score;
    ifs >> b81 >> score;
    book.set(bit_manipulations::toBoard(b81), score);
  }
}

Result Book::think(const board bd) const {
  hand h = PASS;
  int score = -value::VALUE_MAX;
  for (auto next : state::next_states(bd)) {
    int res = -lookup(next);
    if (res > score) {
      h = hand_from_diff(bd, next);
      score = res;
    }
  }
  return Result(h, score);
}

std::vector<board> expand_with_think_impl(const board bd, int depth) {
  if (depth == 0) return std::vector<board>(1, bd);
  std::vector<board> res;
  for (auto next : state::next_states(bd)) {
    auto child = expand_with_think(next, depth-1);
    res.insert(std::end(res), std::begin(child), std::end(child));
  }
  return res;
}

std::vector<board> expand_with_think(const board bd, int depth) {
  if (depth == 0) return std::vector<board>(1, bd);
  if (depth == 1) return expand_with_think_impl(bd, depth);
  hand h = book.think(bd).h;
  if (h == PASS) {
    return expand_with_think_impl(board::reverse_board(bd), depth-1);
  } else {
    return expand_with_think_impl(state::move(bd, h), depth-1);
  }
}

} // namespace book
