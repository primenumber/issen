#include "tree_manager.hpp"

#include <boost/timer.hpp>

#include "bit_manipulations.hpp"

namespace tree_manager {

void tree_manager::step(const board &nx, bool &fliped, std::vector<std::tuple<board, hand>> &ary) {
  hand h = hand_from_diff(bd, nx);
  ary.emplace_back(nx, h);
  play(nx);
  fliped = !fliped;
}

std::tuple<std::vector<std::tuple<board, hand>>, int> tree_manager::endgame_search() {
  std::vector<std::tuple<board, hand>> ary;
  bool fliped = false;
  while (!state::is_gameover(bd)) {
    expand_tree();
    reorder_tree();
    board nx;
    int num;
    std::tie(nx, num) = treesearch::endgame_search(*nd_ptr);
    step(nx, fliped, ary);
    while (!nd_ptr->children.empty()) {
      nx = nd_ptr->children[nd_ptr->cut_pos]->bd;
      step(nx, fliped, ary);
    }
  }
  return std::make_tuple(ary,
      value::fixed_diff_num(std::get<0>(ary.back())) * (fliped ? -1 : 1));
}

std::tuple<board, hand, int> tree_manager::normal_search() {
  expand_tree();
  reorder_tree();
  board nx = nd_ptr->children.front()->bd;
  return std::make_tuple(nx, hand_from_diff(bd, nx), 0);
}

void tree_manager::update_tree(const board &nx) {
  if (nd_ptr->children.empty()) expand_tree();
  for (auto &child : nd_ptr->children) {
    if (child->bd == nx) {
      nd_ptr.reset(child.release());
      break;
    }
  }
}

void tree_manager::reorder_tree() {
  int rem_stones = 64 - bit_manipulations::stone_sum(bd);
  if (rem_stones - dep >= 12) {
    boost::timer t;
    for (dep_rec = dep; t.elapsed() < 0.5 && dep_rec <= dep + 4; ++dep_rec) {
      tree::reorder_recursive(*nd_ptr, value::value, dep_rec);
    }
  } else {
    tree::reorder(*nd_ptr, value::value);
  }
}

void tree_manager::expand_tree() {
  int rem_stones = 64 - bit_manipulations::stone_sum(bd);
  while (dep <= std::max(1, rem_stones / 2)) {
    int ts = tree::expand_recursive(*nd_ptr, dep);
    if (ts > 1000000) break;
    ++dep;
  }
}

} // namespace tree_manager
