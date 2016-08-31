#include "gamesolver.hpp"
#include <iostream>

#include "value.hpp"
#include "state.hpp"
#include "bit_manipulations.hpp"

constexpr int psearch_ordering_th = 57;

uint64_t nodes = 0;
GameSolver::GameSolver(size_t hash_size)
    : tb{table::Table(hash_size), table::Table(hash_size)},
      next_buffer(61), in_buffer(61), out_buffer(61)
{
  for (int i = 0; i <= 60; ++i) {
    next_buffer[i].reserve(64-i);
    in_buffer[i].reserve(64-i);
    out_buffer[i].reserve(64-i);
  }
}

int GameSolver::iddfs(const board &bd) {
  nodes = 0;
  int rem_stones = 64 - bit_manipulations::stone_sum(bd);
  for (int depth = 1200; depth <= rem_stones * 100; depth += 200) {
    tb[0].clear();
    std::cerr << "depth: " << (depth/100) << std::endl;
    int res = iddfs(bd, -value::VALUE_MAX, value::VALUE_MAX, depth, true);
    std::cerr << res << std::endl;
    std::swap(tb[0], tb[1]);
  }
  std::cerr << "full search" << std::endl;
  tb[0].range_max = 64;
  tb[0].clear();
  int res = psearch(bd, -64, 64);
  std::cerr << "nodes total: " << nodes << std::endl;
  return res;
}

bool order_first(const std::pair<int, board> &lhs,
    const std::pair<int, board> &rhs) {
  return lhs.first < rhs.first;
}

bool GameSolver::iddfs_ordering_impl(
    std::vector<std::pair<int, board>> &ary,
    int &alpha, int beta, int &result,
    int depth, bool is_pn, bool &first) {
  std::sort(std::begin(ary), std::end(ary), order_first);
  for (const auto &next : ary) {
    if (!first) {
      result = std::max(result,
          -iddfs(next.second, -alpha-1, -alpha, depth-200, false));
      if (result >= beta) return true;
      if (result <= alpha) continue;
      alpha = result;
    } else {
      first = false;
    }
    result = std::max(result,
        -iddfs(next.second, -beta, -alpha, depth-(first?50:200), is_pn && first));
    if (result >= beta) {
      return true;
    }
    alpha = std::max(alpha, result);
  }
  return false;
}

int GameSolver::iddfs_ordering(
    const board &bd, int alpha, int beta, int depth, bool is_pn) {
  int stone_sum = bit_manipulations::stone_sum(bd);
  bool pass = state::next_states(bd, next_buffer[stone_sum]);
  if (pass) {
    board rev_bd = board::reverse_board(bd);
    if (state::puttable_black(rev_bd) == 0) {
      return value::num_value(bd);
    } else {
      return -iddfs(rev_bd, -beta, -alpha, depth, is_pn);
    }
  }
  std::vector<std::pair<int, board>> &in_hash = in_buffer[stone_sum];
  std::vector<std::pair<int, board>> &out_hash = out_buffer[stone_sum];
  in_hash.clear();
  out_hash.clear();
  if (stone_sum > 54) {
    for (const auto &next : next_buffer[stone_sum]) {
      out_hash.emplace_back(_popcnt64(state::puttable_black(next)), next);
    }
  } else {
    for (const auto &next : next_buffer[stone_sum]) {
      if (auto val_opt = tb[1][next]) {
        in_hash.emplace_back(val_opt->val_max, next);
      } else {
        out_hash.emplace_back(_popcnt64(state::puttable_black(next)), next);
      }
    }
  }
  int result = -value::VALUE_MAX; // fail soft
  bool first = true;
  if (iddfs_ordering_impl(in_hash, alpha, beta, result, depth, is_pn, first))
    return result;
  iddfs_ordering_impl(out_hash, alpha, beta, result, depth, is_pn, first);
  return result;
}

int GameSolver::iddfs_impl(
    const board &bd, int alpha, int beta, int depth, bool is_pn) {
  int stones = bit_manipulations::stone_sum(bd);
  if (stones < 60) {
    return iddfs_ordering(bd, alpha, beta, depth, is_pn);
  } else {
    return psearch_impl(bd, alpha, beta);
  }
}

int GameSolver::iddfs(
    const board &bd, int alpha, int beta, int depth, bool is_pn) {
  ++nodes;
  int stones = bit_manipulations::stone_sum(bd);
  if (depth <= 0) {
    return value::statistic_value(bd);
  }
  if (stones <= 54) {
    if (const auto cache_opt = tb[0][bd]) {
      const auto & cache = *cache_opt;
      if (cache.val_min >= beta) {
        return cache.val_min;
      } else if (cache.val_max <= alpha) {
        return cache.val_max;
      } else {
        table::Range new_ab = cache && table::Range(alpha, beta);
        alpha = new_ab.val_min;
        beta = new_ab.val_max;
        auto res = iddfs_impl(bd, alpha, beta, depth, is_pn);
        tb[0].update(bd, new_ab, res);
        return res;
      } 
    } else {
      auto res = iddfs_impl(bd, alpha, beta, depth, is_pn);
      tb[0].update(bd, table::Range(alpha, beta), res);
      return res;
    }
  } else {
    return iddfs_impl(bd, alpha, beta, depth, is_pn);
  }
}

bool GameSolver::psearch_ordering_impl(
    std::vector<std::pair<int, board>> &ary,
    int &alpha, int beta, int &result, bool &first) {
  std::sort(std::begin(ary), std::end(ary), order_first);
  for (const auto &next : ary) {
    if (!first) {
      result = std::max(result,
          -psearch(next.second, -alpha-1, -alpha));
      if (result >= beta) return true;
      if (result <= alpha) continue;
      alpha = result;
    } else {
      first = false;
    }
    result = std::max(result,
        -psearch(next.second, -beta, -alpha));
    if (result >= beta) {
      return true;
    }
    alpha = std::max(alpha, result);
  }
  return false;
}

int GameSolver::psearch_ordering(const board &bd, int alpha, int beta) {
  int stone_sum = bit_manipulations::stone_sum(bd);
  bool pass = state::next_states(bd, next_buffer[stone_sum]);
  if (pass) {
    board rev_bd = board::reverse_board(bd);
    if (state::puttable_black(rev_bd) == 0) {
      return value::fixed_diff_num(bd);
    } else {
      return -psearch(rev_bd, -beta, -alpha);
    }
  }
  std::vector<std::pair<int, board>> &in_hash = in_buffer[stone_sum];
  std::vector<std::pair<int, board>> &out_hash = out_buffer[stone_sum];
  in_hash.clear();
  out_hash.clear();
  if (stone_sum > 54) {
    for (const auto &next : next_buffer[stone_sum]) {
      out_hash.emplace_back(_popcnt64(state::puttable_black(next)), next);
    }
  } else {
    for (const auto &next : next_buffer[stone_sum]) {
      if (auto val_opt = tb[1][next]) {
        in_hash.emplace_back(val_opt->val_max, next);
      } else {
        out_hash.emplace_back(_popcnt64(state::puttable_black(next)), next);
      }
    }
  }
  int result = -64; // fail soft
  bool first = true;
  if (psearch_ordering_impl(in_hash, alpha, beta, result, first))
    return result;
  psearch_ordering_impl(out_hash, alpha, beta, result, first);
  return result;
}

int GameSolver::psearch_noordering(const board &bd, int alpha, int beta) {
  bool pass = true;
  int result = -64; // fail soft
  uint64_t puttable_bits = ~bit_manipulations::stones(bd);
  for (; puttable_bits; puttable_bits &= puttable_bits-1) {
    const uint64_t bit = puttable_bits & -puttable_bits;
    const uint8_t pos = bit_manipulations::bit_to_pos(bit);
    const board next = state::put_black_at_rev(bd, pos);
    if (next.black() == bd.white()) continue;
    pass = false;
    result = std::max(result, -psearch(next, -beta, -alpha));
    if (result >= beta) {
      return result;
    }
    alpha = std::max(alpha, result);
  }
  if (pass) {
    const board rev_bd = board::reverse_board(bd);
    if (state::puttable_black(rev_bd) == 0) {
      return value::fixed_diff_num(bd);
    } else {
      return -psearch(rev_bd, -beta, -alpha);
    }
  }
  return result;
}

int GameSolver::psearch_leaf(const board &bd) {
  uint64_t pos_bit = ~bit_manipulations::stones(bd);
  int pos = bit_manipulations::bit_to_pos(pos_bit);
  const board nx = state::put_black_at(bd, pos);
  if (nx.white() == bd.white()) {
    const board nx2 = state::put_black_at(board::reverse_board(bd), pos);
    if (nx2.white() == bd.black()) {
      return value::fixed_diff_num(bd);
    } else {
      ++nodes;
      return -value::fixed_diff_num(nx2);
    }
  } else {
    return value::fixed_diff_num(nx);
  }
}

int GameSolver::psearch_impl(const board &bd, int alpha, int beta) {
  int stones = bit_manipulations::stone_sum(bd);
  if (stones <= psearch_ordering_th) {
    return psearch_ordering(bd, alpha, beta);
  } else if (stones <= 62) {
    return psearch_noordering(bd, alpha, beta);
  } else {
    return psearch_leaf(bd);
  }
}

int GameSolver::psearch(const board &bd, int alpha, int beta) {
  ++nodes;
  int stones = bit_manipulations::stone_sum(bd);
  if (stones <= 54) {
    if (const auto cache_opt = tb[0][bd]) {
      const auto & cache = *cache_opt;
      if (cache.val_min >= beta) {
        return cache.val_min;
      } else if (cache.val_max <= alpha) {
        return cache.val_max;
      } else {
        table::Range new_ab = cache && table::Range(alpha, beta);
        alpha = new_ab.val_min;
        beta = new_ab.val_max;
        auto res = psearch_impl(bd, alpha, beta);
        tb[0].update(bd, new_ab, res);
        return res;
      } 
    } else {
      auto res = psearch_impl(bd, alpha, beta);
      tb[0].update(bd, table::Range(alpha, beta), res);
      return res;
    }
  } else {
    return psearch_impl(bd, alpha, beta);
  }
}
