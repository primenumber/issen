#include "treesearch.hpp"

#include <cassert>
#include <algorithm>
#include <iostream>

namespace treesearch {

uint64_t nodes;

int endgame_dfs(const board &bd, int alpha, int beta, bool is_pass = false) {
  uint64_t bits = state::puttable_black(bd);
  ++nodes;
  if (bits != 0) {
    for (auto &nx : state::next_states(bd, bits)) {
      alpha = std::max(alpha,
          -endgame_dfs(nx, -beta, -alpha));
      if (alpha >= beta) return alpha;
    }
    return alpha;
  } else if (is_pass) {
    return value::fixed_diff_num(bd);
  } else {
    return -endgame_dfs(board::reverse_board(bd), -beta, -alpha, true);
  }
}

int endgame_tree_dfs(const tree::node &nd, int alpha, int beta) {
  if (nd.children.empty()) {
    return endgame_dfs(nd.bd, alpha, beta);
  } else {
    ++nodes;
    for (auto &np : nd.children) {
      alpha = std::max(alpha,
          -endgame_tree_dfs(*np, -beta, -alpha));
      if (alpha >= beta) return alpha;
    }
    return alpha;
  }
}

std::tuple<board, int> endgame_search(tree::node &nd) {
  int alpha = -value::VALUE_MAX;
  int beta = value::VALUE_MAX;
  board bd;
  nodes = 0;
  for (auto &np : nd.children) {
    int val = -endgame_tree_dfs(*np, -beta, -alpha);
    if (val > alpha) {
      alpha = val;
      bd = np->bd;
    }
  }
  std::cerr << "nodes: " << nodes << std::endl;
  return std::make_tuple(bd, alpha);
}

} // namespace treesearch
