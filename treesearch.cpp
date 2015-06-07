#include "treesearch.hpp"

#include <cassert>

#include <algorithm>
#include <iostream>
#include <tuple>
#include <vector>
#include <thread>

#include  "tbb/concurrent_queue.h"

#include "state.hpp"
#include "bit_manipulations.hpp"
#include "tree_manager.hpp"

namespace treesearch {

int endgame_dfs(const board &bd, int alpha, int beta, uint64_t &nodes, bool is_pass = false);

int endgame_dfs_sort(const board &bd, int alpha, int beta, uint64_t bits, uint64_t &nodes) {
  std::vector<std::tuple<int, board>> nxv;
  nxv.reserve(_popcnt64(bits));
  for (auto &nx : state::next_states(bd, bits))
    nxv.emplace_back(value::puttable_value(nx), nx);
  using tp = std::tuple<int, board>;
  std::sort(std::begin(nxv), std::end(nxv),
      [](const tp &lhs, const tp &rhs) {
        return std::get<0>(lhs) < std::get<0>(rhs);
      });
  alpha = std::max(alpha,
      -endgame_dfs(std::get<1>(nxv.front()), -beta, -alpha, nodes));
  if (alpha >= beta) return alpha;
  for (int i = 1; i < nxv.size(); ++i) {
    int v = -endgame_dfs(std::get<1>(nxv[i]), -alpha - 1, -alpha, nodes);
    if (v >= beta) return v;
    if (v > alpha) {
      alpha = v;
      alpha = std::max(alpha,
          -endgame_dfs(std::get<1>(nxv[i]), -beta, -v+1, nodes));
      if (alpha >= beta) return alpha;
    }
  }
  return alpha;
}

int endgame_dfs(const board &bd, int alpha, int beta, uint64_t &nodes, bool is_pass) {
  uint64_t bits = state::puttable_black(bd);
  ++nodes;
  if (bits != 0) {
    if (_popcnt64(bits) > 3) return endgame_dfs_sort(bd, alpha, beta, bits, nodes);
    for (; bits != 0; bits &= bits - 1) {
      int pos = bit_manipulations::bit_to_pos(bits & -bits);
      const board nx(state::put_black_at(bd, pos / 8, pos % 8),
          reverse_construct_t());
      alpha = std::max(alpha,
          -endgame_dfs(nx, -beta, -alpha, nodes));
      if (alpha >= beta) return alpha;
    }
    return alpha;
  } else if (is_pass) {
    return value::fixed_diff_num(bd);
  } else {
    return -endgame_dfs(board::reverse_board(bd), -beta, -alpha, nodes, true);
  }
}

int leaf_table_update(const board &bd, int alpha, int beta,
    const std::vector<board> &vb, std::vector<std::tuple<int, int>> &vv,
    uint64_t &nodes) {
  int index = std::lower_bound(std::begin(vb), std::end(vb), bd)
      - std::begin(vb);
  int lower, upper;
  std::tie(lower, upper) = vv[index];
  if (lower >= beta) return lower;
  if (upper <= alpha) return upper;
  int val = endgame_dfs(bd, std::max(lower, alpha), std::min(upper, beta), nodes);
  if (val <= alpha) std::get<1>(vv[index]) = std::max(lower, alpha);
  else if (val >= beta) std::get<0>(vv[index]) = std::min(upper, beta);
  else vv[index] = std::make_tuple(val, val);
  return val;
}

int endgame_tree_dfs(tree::node &nd, int alpha, int beta,
    const std::vector<board> &vb, std::vector<std::tuple<int, int>> &vv,
    uint64_t &nodes) {
  if (nd.children.empty()) {
    return leaf_table_update(nd.bd, alpha, beta, vb, vv, nodes);
  } else {
    ++nodes;
    nd.cut_pos = 0;
    tree::node &bd_first = *nd.children.front();
    alpha = std::max(alpha,
        -endgame_tree_dfs(bd_first, -beta, -alpha, vb, vv, nodes));
    if (alpha >= beta) return alpha;
    for (int i = 1; i < nd.children.size(); ++i) {
      auto &nx = *(nd.children[i]);
      int nws_v = -endgame_tree_dfs(nx, -alpha - 1, -alpha, vb, vv, nodes);
      if (nws_v >= beta) {
        nd.cut_pos = i;
        return nws_v;
      }
      if (nws_v > alpha) {
        int value = -endgame_tree_dfs(nx, -beta, -nws_v+1, vb, vv, nodes);
        if (value > alpha) {
          alpha = value;
          nd.cut_pos = i;
        }
        if (alpha >= beta) {
          return alpha;
        }
      }
    }
    return alpha;
  }
}

std::tuple<board, int> endgame_search_fromleaf(const board &bd, uint64_t &nodes) {
  int alpha = -value::VALUE_MAX;
  int beta = value::VALUE_MAX;
  board opt;
  nodes = 0;
  for (auto &nx : state::next_states(bd)) {
    int val = -endgame_dfs(nx, -beta, -alpha, nodes);
    if (val > alpha) {
      alpha = val;
      opt = nx;
    }
  }
  return std::make_tuple(opt, alpha);
}

std::tuple<board, int> endgame_search(tree::node &nd) {
  uint64_t nodes = 0;
  if (nd.children.empty()) return endgame_search_fromleaf(nd.bd, nodes);
  auto vb = tree::unique_leafs(nd);
  std::vector<std::tuple<int, int>> vv(vb.size(), std::make_tuple(-value::VALUE_MAX, value::VALUE_MAX));
  int alpha = -endgame_tree_dfs(*nd.children.front(), -value::VALUE_MAX, value::VALUE_MAX, vb, vv, nodes);
  int beta = value::VALUE_MAX;
  board bd = nd.children.front()->bd;
  for (int i = 1; i < nd.children.size(); ++i) {
    auto &nx = *(nd.children[i]);
    int nws_v = -endgame_tree_dfs(nx, -alpha-1, -alpha, vb, vv, nodes);
    if (nws_v > alpha) {
      int val = -endgame_tree_dfs(nx, -beta, -nws_v+1, vb, vv, nodes);
      if (val > alpha) {
        alpha = val;
        bd = nx.bd;
        nd.cut_pos = i;
      }
    }
  }
  std::cerr << "nodes: " << nodes << std::endl;
  return std::make_tuple(bd, alpha);
}

} // namespace treesearch
