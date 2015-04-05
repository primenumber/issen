#pragma once

#include <memory>

#include "board.hpp"
#include "hand.hpp"
#include "tree.hpp"
#include "treesearch.hpp"

namespace tree_manager {

class tree_manager {
 public:
  tree_manager(const board &bd, bool is_black)
    : bd(bd), is_black(is_black), nd_ptr(std::make_unique<tree::node>(bd)),
      dep(0), dep_rec(0) {
  }
  //std::tuple<board, hand, int> normal_search() {}
  std::tuple<board, hand, int> endgame_search() {
    expand_tree();
    reorder_tree();
    board nx;
    int num;
    std::tie(nx, num) = treesearch::endgame_search(*nd_ptr);
    hand h = hand_from_diff(bd, nx);
    play(nx);
    return std::make_tuple(nx, h, num);
  }
  void play(const board &nx) {
    update_tree(nx);
    bd = nx;
    is_black = !is_black;
    --dep;
    --dep_rec;
  }
  board get_board() const { return bd; }
 private:
  board bd;
  bool is_black;
  std::unique_ptr<tree::node> nd_ptr;
  int dep, dep_rec;
  void update_tree(const board &bd);
  void reorder_tree();
  void expand_tree();
};

} // namespace tree_manager
