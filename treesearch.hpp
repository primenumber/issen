#pragma once
#include <memory>
#include <vector>

#include "board.hpp"
#include "state.hpp"
#include "value.hpp"

namespace treesearch {

struct node {
  const board bd;
  std::vector<std::unique_ptr<node>> children;
  std::vector<int> value;
  node(const board &bd) : bd(bd) {}
};

void expand(node &);

void expand_recursive(node &, int);

template <typename Func>
int tree_negamax(node &nd, int depth, const Func &func) {
  return tree_negaalpha(nd, depth,
      -value::VALUE_MAX, value::VALUE_MAX, func);
}

template <typename Func>
std::pair<int, board> tree_negamax_unlimited(node &nd, const Func &func) {
  return tree_negaalpha_unlimited(
      nd, -value::VALUE_MAX, value::VALUE_MAX, func);
}

#include "treesearch_impl.hpp"

} // namespace treesearch
