#include "gamesolver.hpp"
#include <iostream>

#include "value.hpp"
#include "state.hpp"
#include "bit_manipulations.hpp"

uint64_t nodes = 0;

int GameSolver::iddfs(const board &bd) {
  nodes = 0;
  rem_stones = 64 - bit_manipulations::stone_sum(bd);
  int old_rems = rem_stones;
  for (rem_stones -= 6; old_rems - rem_stones <= std::min(old_rems/2, 12); --rem_stones) {
    tb[0].clear();
    std::cerr << "rem: " << rem_stones << std::endl;
    dfs(bd, -value::VALUE_MAX, value::VALUE_MAX);
    std::swap(tb[0], tb[1]);
  }
  std::cerr << "full search" << std::endl;
  rem_stones = -1;
  tb[0].clear();
  int res = dfs(bd, -value::VALUE_MAX, value::VALUE_MAX);
  std::cerr << "nodes total: " << nodes << std::endl;
  return res;
}

bool order_first(const std::pair<int, board> &lhs,
    const std::pair<int, board> &rhs) {
  return lhs.first < rhs.first;
}

int GameSolver::dfs_ordering(const board &bd, int alpha, int beta) {
  uint64_t puttable_bits = state::puttable_black(bd);
  bool pass = (puttable_bits == 0);
  std::vector<std::pair<int, board>> in_hash, out_hash;
  if (pass && state::puttable_black(board::reverse_board(bd)) == 0) {
    return value::num_value(bd);
  }
  const auto nexts = state::next_states(bd, puttable_bits);
  for (const auto &next : nexts) {
    if (auto val_opt = tb[1][next]) {
      in_hash.emplace_back(val_opt->val_max, next);
    } else {
      out_hash.emplace_back(_popcnt64(state::puttable_black(next)), next);
    }
  }
  int result = -value::VALUE_MAX; // fail soft
  bool first = true;
  std::sort(std::begin(in_hash), std::end(in_hash), order_first);
  for (const auto &next : in_hash) {
    if (!first) {
      result = std::max(result,
          -dfs(next.second, -alpha-1, -alpha));
      if (result >= beta) return result;
      if (result <= alpha) continue;
      alpha = result;
    } else {
      first = false;
    }
    result = std::max(result,
        -dfs(next.second, -beta, -alpha));
    if (result >= beta) {
      return result;
    }
    alpha = std::max(alpha, result);
  }
  std::sort(std::begin(out_hash), std::end(out_hash), order_first);
  for (const auto &next : out_hash) {
    if (!first) {
      result = std::max(result,
          -dfs(next.second, -alpha-1, -alpha));
      if (result >= beta) return result;
      if (result <= alpha) continue;
      alpha = result;
    } else {
      first = false;
    }
    result = std::max(result,
        -dfs(next.second, -beta, -alpha));
    if (result >= beta) {
      return result;
    }
    alpha = std::max(alpha, result);
  }
  return result;
}

int GameSolver::dfs_noordering(const board &bd, int alpha, int beta) {
  uint64_t puttable_bits = state::puttable_black(bd);
  bool pass = (puttable_bits == 0);
  if (pass) {
    const board rev_bd = board::reverse_board(bd);
    if (state::puttable_black(rev_bd) == 0) {
      return value::num_value(bd);
    } else {
      return -dfs(rev_bd, -beta, -alpha);
    }
  }
  int result = -value::VALUE_MAX; // fail soft
  for (; puttable_bits; puttable_bits &= puttable_bits-1) {
    const uint64_t bit = puttable_bits & -puttable_bits;
    const uint8_t pos = bit_manipulations::bit_to_pos(bit);
    const board next = state::put_black_at_rev(bd, pos);
    result = std::max(result, -dfs(next, -beta, -alpha));
    if (result >= beta) {
      return result;
    }
    alpha = std::max(alpha, result);
  }
  return result;
}

int GameSolver::dfs_noordering2(const board &bd, int alpha, int beta) {
  bool pass = true;
  int result = -value::VALUE_MAX; // fail soft
  uint64_t puttable_bits = ~bit_manipulations::stones(bd);
  for (; puttable_bits; puttable_bits &= puttable_bits-1) {
    const uint64_t bit = puttable_bits & -puttable_bits;
    const uint8_t pos = bit_manipulations::bit_to_pos(bit);
    const board next = state::put_black_at_rev(bd, pos);
    if (next.black() == bd.white()) continue;
    pass = false;
    result = std::max(result, -dfs(next, -beta, -alpha));
    if (result >= beta) {
      return result;
    }
    alpha = std::max(alpha, result);
  }
  if (pass) {
    const board rev_bd = board::reverse_board(bd);
    if (state::puttable_black(rev_bd) == 0) {
      return value::num_value(bd);
    } else {
      return -dfs(rev_bd, -beta, -alpha);
    }
  }
  return result;
}

int GameSolver::dfs_leaf(const board &bd) {
  uint64_t pos_bit = ~bit_manipulations::stones(bd);
  int pos = bit_manipulations::bit_to_pos(pos_bit);
  const board nx = state::put_black_at(bd, pos);
  if (nx.white() == bd.white()) {
    const board nx2 = state::put_black_at(board::reverse_board(bd), pos);
    if (nx2.white() == bd.black()) {
      return value::num_value(bd);
    } else {
      return -value::num_value(nx2);
    }
  } else {
    return value::num_value(nx);
  }
}

int GameSolver::dfs_impl(const board &bd, int alpha, int beta) {
  int stones = bit_manipulations::stone_sum(bd);
  if (stones <= 56) {
    return dfs_ordering(bd, alpha, beta);
  //} else if (stones <= 59) {
    //return dfs_noordering(bd, alpha, beta);
  } else if (stones <= 62) {
    return dfs_noordering2(bd, alpha, beta);
  } else {
    return dfs_leaf(bd);
  }
}

int GameSolver::dfs(const board &bd, int alpha, int beta) {
  ++nodes;
  int stones = bit_manipulations::stone_sum(bd);
  if (64 - stones <= rem_stones) {
    return value::statistic_value(bd);
  }
  if (stones <= 56) {
    if (const auto cache_opt = tb[0][bd]) {
      const auto & cache = *cache_opt;
      if (cache.val_min >= beta) {
        return cache.val_min;
      } else {
        table::Range new_ab = cache && table::Range(alpha, beta);
        if (new_ab.val_min < new_ab.val_max) {
          alpha = new_ab.val_min;
          beta = new_ab.val_max;
          auto res = dfs_impl(bd, alpha, beta);
          tb[0].update(bd, new_ab, res);
          return res;
        }
        return cache.val_max;
      } 
    } else {
      auto res = dfs_impl(bd, alpha, beta);
      tb[0].update(bd, table::Range(alpha, beta), res);
      return res;
    }
  } else {
    return dfs_impl(bd, alpha, beta);
  }
}
