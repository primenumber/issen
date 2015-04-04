#pragma once
#include <memory>
#include <tuple>
#include <vector>

#include "board.hpp"
#include "hand.hpp"
#include "tree.hpp"
#include "value.hpp"

namespace treesearch {

template <typename Func>
int tree_negamax(tree::node &nd, int depth, const Func &func) {
  return tree_negaalpha(nd, depth,
      -value::VALUE_MAX, value::VALUE_MAX, func);
}

std::tuple<board, int> endgame_search(tree::node &);

} // namespace treesearch
