#include "iddfs.hpp"
#include "value.hpp"
#include "state.hpp"
#include "bit_manipulations.hpp"

uint64_t cnt = 0;
uint64_t nodes = 0;
void Table::update(
    const board &bd, const Range range, const int32_t value) {
  ++cnt;
  uint64_t h = bd_hash(bd);
  auto &p = table[h % hash_size];
  if (range.val_min < value && value < range.val_max) {
    p = std::make_pair(bd, Range(value));
  } else {
    if (p.first == bd) {
      if (value >= range.val_max) {
        p.second.update_min(range.val_max);
      } else if (value <= range.val_min) {
        p.second.update_max(range.val_min);
      }
    } else {
      Range r;
      if (value >= range.val_max) {
        r.update_min(range.val_max);
      } else if (value <= range.val_min) {
        r.update_max(range.val_min);
      }
      p = std::make_pair(bd, r);
    }
  }
}

int GameSolver::iddfs(const board &bd) {
  cnt = 0;
  nodes = 0;
  for (max_depth = 6; max_depth < 12; ++max_depth) {
    table[0].clear();
    //std::cerr << "depth: " << max_depth << std::endl;
    dfs(bd, max_depth, -value::VALUE_MAX, value::VALUE_MAX);
    std::swap(table[0], table[1]);
  }
  //std::cerr << "depth: inf" << std::endl;
  max_depth = 120;
  table[0].clear();
  int res = dfs(bd, max_depth, -value::VALUE_MAX, value::VALUE_MAX);
  //std::cerr << "hash update: " << cnt << std::endl;
  //std::cerr << "nodes total: " << nodes << std::endl;
  return res;
}

bool order_first(const std::pair<int, board> &lhs,
    const std::pair<int, board> &rhs) {
  return lhs.first < rhs.first;
}

int GameSolver::dfs_ordering(
    const board &bd, int depth, int alpha, int beta) {
  uint64_t puttable_bits = state::puttable_black(bd);
  bool pass = (puttable_bits == 0);
  std::vector<std::pair<int, board>> in_hash, out_hash;
  if (pass && state::puttable_black(board::reverse_board(bd)) == 0) {
    return value::num_value(bd);
  }
  const auto nexts = state::next_states(bd, puttable_bits);
  for (const auto &next : nexts) {
    if (auto val_opt = table[1][next]) {
      in_hash.emplace_back(val_opt->val_min, next);
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
          -dfs(next.second, depth-1, -alpha-1, -alpha));
      if (result >= beta) return result;
      if (result <= alpha) continue;
      alpha = result;
    } else {
      first = false;
    }
    result = std::max(result,
        -dfs(next.second, depth-1, -beta, -alpha));
    if (result >= beta) {
      return result;
    }
    alpha = std::max(alpha, result);
  }
  std::sort(std::begin(out_hash), std::end(out_hash), order_first);
  for (const auto &next : out_hash) {
    if (!first) {
      result = std::max(result,
          -dfs(next.second, depth-1, -alpha-1, -alpha));
      if (result >= beta) return result;
      if (result <= alpha) continue;
      alpha = result;
    } else {
      first = false;
    }
    result = std::max(result,
        -dfs(next.second, depth-1, -beta, -alpha));
    if (result >= beta) {
      return result;
    }
    alpha = std::max(alpha, result);
  }
  return result;
}

int GameSolver::dfs_noordering(
    const board &bd, int depth, int alpha, int beta) {
  uint64_t puttable_bits = state::puttable_black(bd);
  bool pass = (puttable_bits == 0);
  if (pass) {
    const board rev_bd = board::reverse_board(bd);
    if (state::puttable_black(rev_bd) == 0) {
      return value::num_value(bd);
    } else {
      return -dfs(rev_bd, depth-1, -beta, -alpha);
    }
  }
  int result = -value::VALUE_MAX; // fail soft
  for (; puttable_bits; puttable_bits &= puttable_bits-1) {
    const uint64_t bit = puttable_bits & -puttable_bits;
    const uint8_t pos = bit_manipulations::bit_to_pos(bit);
    const board next = state::put_black_at_rev(bd, pos / 8, pos & 7);
    result = std::max(result, -dfs(next, depth-1, -beta, -alpha));
    if (result >= beta) {
      return result;
    }
    alpha = std::max(alpha, result);
  }
  return result;
}

int GameSolver::dfs_noordering2(
    const board &bd, int depth, int alpha, int beta) {
  bool pass = true;
  int result = -value::VALUE_MAX; // fail soft
  uint64_t puttable_bits = ~(bd.black() | bd.white());
  for (; puttable_bits; puttable_bits &= puttable_bits-1) {
    const uint64_t bit = puttable_bits & -puttable_bits;
    const uint8_t pos = bit_manipulations::bit_to_pos(bit);
    const board next = state::put_black_at_rev(bd, pos / 8, pos & 7);
    if (next.black() == bd.white()) continue;
    pass = false;
    result = std::max(result, -dfs(next, depth-1, -beta, -alpha));
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
      return -dfs(rev_bd, depth-1, -beta, -alpha);
    }
  }
  return result;
}

int GameSolver::dfs_leaf(const board &bd) {
  uint64_t pos_bit = ~(bd.black() | bd.white());
  int pos = bit_manipulations::bit_to_pos(pos_bit);
  const board nx = state::put_black_at(bd, pos / 8, pos & 7);
  if (nx.white() == bd.white()) {
    const board nx2 = state::put_black_at(board::reverse_board(bd), pos / 8, pos & 7);
    if (nx2.white() == bd.black()) {
      return value::num_value(bd);
    } else {
      return -value::num_value(nx2);
    }
  } else {
    return value::num_value(nx);
  }
}

int GameSolver::dfs_impl(
    const board &bd, int depth, int alpha, int beta) {
  int stones = _popcnt64(bd.black() | bd.white());
  if (stones <= 56) {
    return dfs_ordering(bd, depth, alpha, beta);
  //} else if (stones <= 59) {
    //return dfs_noordering(bd, depth, alpha, beta);
  } else if (stones <= 62) {
    return dfs_noordering2(bd, depth, alpha, beta);
  } else {
    return dfs_leaf(bd);
  }
}

int GameSolver::dfs(const board &bd, int depth,
    int alpha, int beta) {
  ++nodes;
  if (depth == 0) {
    return value::value(bd);
  }
  int stones = _popcnt64(bd.black() | bd.white());
  if (stones <= 56) {
    if (const auto cache_opt = table[0][bd]) {
      const auto & cache = *cache_opt;
      if (cache.val_min >= beta) {
        return cache.val_min;
      } else {
        Range new_ab = cache && Range(alpha, beta);
        if (new_ab.val_min < new_ab.val_max) {
          alpha = new_ab.val_min;
          beta = new_ab.val_max;
          auto res = dfs_impl(bd, depth, alpha, beta);
          table[0].update(bd, new_ab, res);
          return res;
        }
        return cache.val_max;
      } 
    } else {
      auto res = dfs_impl(bd, depth, alpha, beta);
      table[0].update(bd, Range(alpha, beta), res);
      return res;
    }
  } else {
    return dfs_impl(bd, depth, alpha, beta);
  }
}
