#include "treesearch.hpp"

#include <algorithm>
#include <iostream>
#include <tuple>
#include <vector>

namespace treesearch {

uint64_t nodes;

int endgame_dfs(const board &bd, int alpha, int beta, bool is_pass = false);

int endgame_dfs_sort(const board &bd, int alpha, int beta, uint64_t bits) {
  std::vector<std::tuple<int, board>> nxv;
  nxv.reserve(_popcnt64(bits));
  for (auto &nx : state::next_states(bd, bits))
    nxv.emplace_back(value::value(nx), nx);
  using tp = std::tuple<int, board>;
  std::sort(std::begin(nxv), std::end(nxv),
      [](const tp &lhs, const tp &rhs) {
        return std::get<0>(lhs) < std::get<0>(rhs);
      });
  alpha = std::max(alpha,
      -endgame_dfs(std::get<1>(nxv.front()), -beta, -alpha));
  if (alpha >= beta) return alpha;
  for (int i = 1; i < nxv.size(); ++i) {
    int v = -endgame_dfs(std::get<1>(nxv[i]), -alpha - 1, -alpha);
    if (v >= beta) return v;
    if (v > alpha) {
      alpha = v;
      alpha = std::max(alpha,
          -endgame_dfs(std::get<1>(nxv[i]), -beta, -alpha));
      if (alpha >= beta) return alpha;
    }
  }
  return alpha;
}

int endgame_dfs(const board &bd, int alpha, int beta, bool is_pass) {
  uint64_t bits = state::puttable_black(bd);
  ++nodes;
  if (bits != 0) {
    if (_popcnt64(bits) > 4) return endgame_dfs_sort(bd, alpha, beta, bits);
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

int leaf_table_update(const board &bd, int alpha, int beta,
    const std::vector<board> &vb, std::vector<std::tuple<int, int>> &vv) {
  int index = std::lower_bound(std::begin(vb), std::end(vb), bd)
      - std::begin(vb);
  int lower, upper;
  std::tie(lower, upper) = vv[index];
  if (lower >= beta) return lower;
  if (upper <= alpha) return upper;
  int val = endgame_dfs(bd, std::max(lower, alpha), std::min(upper, beta));
  if (val <= alpha) std::get<1>(vv[index]) = std::max(lower, alpha);
  else if (val >= beta) std::get<0>(vv[index]) = std::min(upper, beta);
  else vv[index] = std::make_tuple(val, val);
  return val;
}
int endgame_tree_dfs(const tree::node &nd, int alpha, int beta,
    const std::vector<board> &vb, std::vector<std::tuple<int, int>> &vv) {
  if (nd.children.empty()) {
    return leaf_table_update(nd.bd, alpha, beta, vb, vv);
  } else {
    ++nodes;
    for (auto &np : nd.children) {
      alpha = std::max(alpha,
          -endgame_tree_dfs(*np, -beta, -alpha, vb, vv));
      if (alpha >= beta) return alpha;
    }
    return alpha;
  }
}

std::tuple<board, int> endgame_search(tree::node &nd) {
  int alpha = -value::VALUE_MAX;
  int beta = value::VALUE_MAX;
  auto vb = tree::unique_leafs(nd);
  std::vector<std::tuple<int, int>> vv(vb.size(), std::make_tuple(-value::VALUE_MAX, value::VALUE_MAX));
  board bd;
  nodes = 0;
  for (auto &np : nd.children) {
    int val = -endgame_tree_dfs(*np, -beta, -alpha, vb, vv);
    if (val > alpha) {
      alpha = val;
      bd = np->bd;
    }
  }
  std::cerr << "nodes: " << nodes << std::endl;
  return std::make_tuple(bd, alpha);
}

} // namespace treesearch
