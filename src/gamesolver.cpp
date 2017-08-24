#include "gamesolver.hpp"
#include <iostream>
#include <iterator>
#include <atomic>
#include <thread>
#include <future>

#include "value.hpp"
#include "state.hpp"
#include "bit_manipulations.hpp"

constexpr int psearch_ordering_th = 57;

std::atomic<uint64_t> nodes(0);
GameSolver::GameSolver(size_t hash_size)
    : tb{table::Table(hash_size), table::Table(hash_size)} {}

int GameSolver::solve(const board &bd, const GameSolverParam solver_param) {
  param = solver_param;
  nodes = 0;
  int rem_stones = 64 - bit_manipulations::stone_sum(bd);
  int res = 0;
  for (int depth = std::min(rem_stones * param.iddfs_pv_extension, 1000); depth <= rem_stones * 100 - 1200; depth += 100) {
    tb[0].clear();
    if (param.debug) std::cerr << "depth: " << (depth/100) << std::endl;
    res = iddfs(bd, -value::VALUE_MAX, value::VALUE_MAX, depth, true);
    if (param.debug) std::cerr << res << std::endl;
    std::swap(tb[0], tb[1]);
  }
  if (!param.perfect) {
    return res / 100;
  }
  tb[0].range_max = 64;
  tb[0].clear();
  if (param.debug) std::cerr << "full search" << std::endl;
  res = psearch(bd, -64, 64, param.parallel_search ? 2 : 0);
  if (param.debug) {
    std::cerr << "nodes total: " << nodes.load() << std::endl;
    std::cerr << "hash update: " << (tb[0].update_num() + tb[1].update_num()) << std::endl;
    std::cerr << "hash conflict: " << (tb[0].conflict_num() + tb[1].conflict_num()) << std::endl;
  }
  return res;
}

bool order_impl(const std::tuple<int, board> &lhs,
    const std::tuple<int, board> &rhs) {
  using std::get;
  return get<0>(lhs) < get<0>(rhs);
}

bool order_impl(const std::tuple<int, int, board> &lhs,
    const std::tuple<int, int, board> &rhs) {
  using std::get;
  return get<0>(lhs) == get<0>(rhs) ? get<1>(lhs) < get<1>(rhs) : get<0>(lhs) < get<0>(rhs);
}

board get_board(const std::tuple<int, board> &t) {
  return std::get<1>(t);
}

board get_board(const std::tuple<int, int, board> &t) {
  return std::get<2>(t);
}

template <typename InputIt>
bool GameSolver::iddfs_ordering_impl(
    InputIt next_first, InputIt next_last,
    int &alpha, int beta, int &result,
    int depth, bool is_pn, bool &first) {
  using T = typename std::iterator_traits<InputIt>::value_type;
  std::sort(next_first, next_last, (bool(*)(const T&, const T&))order_impl);
  for (auto itr = next_first; itr != next_last; ++itr) {
    const auto &next = *itr;
    if (!first) {
      result = std::max(result,
          -iddfs(get_board(next), -alpha-1, -alpha, depth-100, false));
      if (result >= beta) return true;
      if (result <= alpha) continue;
      alpha = result;
    } else {
      first = false;
    }
    result = std::max(result,
        -iddfs(get_board(next), -beta, -alpha, depth-(first?param.iddfs_pv_extension:100), is_pn && first));
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
  std::array<board, 60> next_buffer;
  int puttable_count = state::next_states(bd, next_buffer);
  if (puttable_count == 0) {
    board rev_bd = board::reverse_board(bd);
    if (state::puttable_black(rev_bd) == 0) {
      return value::num_value(bd);
    } else {
      return -iddfs(rev_bd, -beta, -alpha, depth, is_pn);
    }
  }
  std::array<std::tuple<int, int, board>, 60> in_hash;
  std::array<std::tuple<int, board>, 60> out_hash;
  int count_in = 0;
  int count_out = 0;
  if (stone_sum > 54) {
    for (int i = 0; i < puttable_count; ++i) {
      const auto &next = next_buffer[i];
      out_hash[count_out++] = std::make_tuple(_popcnt64(state::puttable_black(next)), next);
    }
  } else {
    for (int i = 0; i < puttable_count; ++i) {
      const auto &next = next_buffer[i];
      if (auto val_opt = tb[1][next]) {
        in_hash[count_in++] = std::make_tuple(val_opt->val_max, val_opt->val_min, next);
      } else {
        out_hash[count_out++] = std::make_tuple(_popcnt64(state::puttable_black(next)), next);
      }
    }
  }
  int result = -value::VALUE_MAX; // fail soft
  bool first = true;
  if (iddfs_ordering_impl(std::begin(in_hash), std::begin(in_hash) + count_in,
        alpha, beta, result, depth, is_pn, first))
    return result;
  alpha = std::max(alpha, result);
  iddfs_ordering_impl(std::begin(out_hash), std::begin(out_hash) + count_out,
      alpha, beta, result, depth, is_pn, first);
  return result;
}

int GameSolver::iddfs_impl(
    const board &bd, int alpha, int beta, int depth, bool is_pn) {
  int stones = bit_manipulations::stone_sum(bd);
  if (stones < 60) {
    return iddfs_ordering(bd, alpha, beta, depth, is_pn);
  } else {
    return psearch_impl(bd, alpha, beta, is_pn) * 100;
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

int GameSolver::null_window_search_impl(const board &bd, int alpha, int beta, int result, int ybwc_depth) {
  result = std::max(result, -psearch(bd, -alpha-1, -alpha, ybwc_depth));
  if (result >= beta) return result;
  if (result > alpha) {
    alpha = result;
    return std::max(result, -psearch(bd, -beta, -alpha, ybwc_depth));
  }
  return result;
}

int GameSolver::psearch_ybwc(const board &bd, int alpha, int beta, int ybwc_depth) {
  std::array<board, 60> next_buffer;
  int puttable_count = state::next_states(bd, next_buffer);
  if (puttable_count == 0) {
    board rev_bd = board::reverse_board(bd);
    if (state::puttable_black(rev_bd) == 0) {
      return value::fixed_diff_num(bd);
    } else {
      return -psearch(rev_bd, -beta, -alpha, ybwc_depth);
    }
  }
  int pv_index = 0;
  table::Range pv_range(-value::VALUE_MAX, -value::VALUE_MAX);
  bool is_in_hash = false;
  int puttable_min = 64;
  for (int i = 0; i < puttable_count; ++i) {
    const auto &next = next_buffer[i];
    if (auto val_opt = tb[1][next]) {
      if (!is_in_hash) {
        is_in_hash = true;
        pv_range = *val_opt;
        pv_index = i;
      } else if (*val_opt < pv_range) {
        pv_range = *val_opt;
        pv_index = i;
      }
    } else if (!is_in_hash) {
      int count = _popcnt64(state::puttable_black(next));
      if (puttable_min > count) {
        pv_index = i;
        puttable_min = count;
      }
    }
  }
  int result = -psearch(next_buffer[pv_index], -beta, -alpha, ybwc_depth);
  if (result >= beta) return result;
  alpha = std::max(alpha, result);
  std::vector<std::future<int>> vf;
  for (int i = 0; i < puttable_count; ++i) {
    if (i == pv_index) continue;
    const board next = next_buffer[i];
    vf.push_back(std::async(std::launch::async, &GameSolver::null_window_search_impl, this, next, alpha, beta, result, ybwc_depth-1));
  }
  for (auto &&f : vf) {
    result = std::max(result, f.get());
  }
  return result;
}

template <typename InputIt>
bool GameSolver::psearch_ordering_impl(
    InputIt next_first, InputIt next_last,
    int &alpha, int beta, int &result, bool &first, int ybwc_depth) {
  using T = typename std::iterator_traits<InputIt>::value_type;
  std::sort(next_first, next_last, (bool(*)(const T&, const T&))order_impl);
  for (auto itr = next_first; itr != next_last; ++itr) {
    const auto &next = *itr;
    if (!first) {
      result = std::max(result,
          -psearch(get_board(next), -alpha-1, -alpha, ybwc_depth));
      if (result >= beta) return true;
      if (result <= alpha) continue;
      alpha = result;
    } else {
      first = false;
    }
    result = std::max(result,
        -psearch(get_board(next), -beta, -alpha, ybwc_depth));
    if (result >= beta) {
      return true;
    }
    alpha = std::max(alpha, result);
  }
  return false;
}

int GameSolver::psearch_ordering(const board &bd, int alpha, int beta, int ybwc_depth) {
  std::array<board, 60> next_buffer;
  int puttable_count = state::next_states(bd, next_buffer);
  if (puttable_count == 0) {
    board rev_bd = board::reverse_board(bd);
    if (state::puttable_black(rev_bd) == 0) {
      return value::fixed_diff_num(bd);
    } else {
      return -psearch(rev_bd, -beta, -alpha, ybwc_depth);
    }
  }
  std::array<std::tuple<int, int, board>, 60> in_hash;
  std::array<std::tuple<int, board>, 60> out_hash;
  int count_in = 0;
  int count_out = 0;
  for (int i = 0; i < puttable_count; ++i) {
    const auto &next = next_buffer[i];
    if (auto val_opt = tb[1][next]) {
      in_hash[count_in++] = std::make_tuple(val_opt->val_max, val_opt->val_min, next);
    } else {
      out_hash[count_out++] = std::make_tuple(_popcnt64(state::puttable_black(next)), next);
    }
  }
  int result = -64; // fail soft
  bool first = true;
  if (psearch_ordering_impl(std::begin(in_hash), std::begin(in_hash) + count_in,
        alpha, beta, result, first, ybwc_depth))
    return result;
  alpha = std::max(alpha, result);
  psearch_ordering_impl(std::begin(out_hash), std::begin(out_hash) + count_out,
      alpha, beta, result, first, ybwc_depth);
  return result;
}

int GameSolver::psearch_static_ordering(const board &bd, int alpha, int beta) {
  std::array<board, 60> next_buffer;
  int puttable_count = state::next_states(bd, next_buffer);
  if (puttable_count == 0) {
    board rev_bd = board::reverse_board(bd);
    if (state::puttable_black(rev_bd) == 0) {
      return value::fixed_diff_num(bd);
    } else {
      return -psearch_nohash(rev_bd, -beta, -alpha);
    }
  }
  std::array<std::tuple<int, int>, 60> index_ary;
  int count_ary = 0;
  for (int i = 0; i < puttable_count; ++i) {
    const auto &next = next_buffer[i];
    index_ary[count_ary++] = std::make_tuple(_popcnt64(state::puttable_black(next)), i);
  }
  std::sort(std::begin(index_ary), std::begin(index_ary) + puttable_count,
      [](const auto &lhs, const auto &rhs) { return std::get<0>(lhs) < std::get<0>(rhs); });
  int result = -psearch_nohash(next_buffer[std::get<1>(index_ary[0])], -beta, -alpha);
  if (result >= beta) return result;
  alpha = std::max(alpha, result);
  for (int i = 1; i < puttable_count; ++i) {
    result = std::max(result, -psearch_nohash(next_buffer[std::get<1>(index_ary[i])], -alpha-1, -alpha));
    if (result >= beta) return result;
    if (result <= alpha) continue;
    alpha = result;
    result = std::max(result, -psearch_nohash(next_buffer[std::get<1>(index_ary[i])], -beta, -alpha));
    if (result >= beta) return result;
    alpha = std::max(alpha, result);
  }
  return result;
}

int GameSolver::psearch_noordering(const board &bd, int alpha, int beta) {
  bool pass = true;
  int result = -64; // fail soft
  uint64_t puttable_bits = ~bit_manipulations::stones(bd);
  for (; puttable_bits; puttable_bits = _blsr_u64(puttable_bits)) {
    const uint64_t bit = _blsi_u64(puttable_bits);
    const uint8_t pos = bit_manipulations::bit_to_pos(bit);
    const board next = state::put_black_at_rev(bd, pos);
    if (next.black() == bd.white()) continue;
    pass = false;
    result = std::max(result, -psearch_nohash(next, -beta, -alpha));
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
      return -psearch_nohash(rev_bd, -beta, -alpha);
    }
  }
  return result;
}

int GameSolver::psearch_leaf(const board &bd) {
  ++nodes;
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

int GameSolver::psearch_2(const board &bd, int alpha, int beta) {
  const uint64_t black = bd.black();
  const uint64_t white = bd.white();
  const uint64_t puttable_bits = ~bit_manipulations::stones(bd);
  const uint64_t bit1 = _blsi_u64(puttable_bits);
  const uint8_t pos1 = bit_manipulations::bit_to_pos(bit1);
  board next = state::put_black_at_rev(bd, pos1);
  const uint64_t bit2 = _blsr_u64(puttable_bits);
  const uint8_t pos2 = bit_manipulations::bit_to_pos(bit2);
  if (next.black() == white) {
    next = state::put_black_at_rev(bd, pos2);
    if (next.black() == white) {
      ++nodes;
      const board rev_bd = board::reverse_board(bd);
      next = state::put_black_at_rev(rev_bd, pos1);
      if (next.black() == black) {
        next = state::put_black_at_rev(rev_bd, pos2);
        if (next.black() == black) {
          return value::fixed_diff_num(bd);
        } else {
          return psearch_leaf(next);
        }
      } else {
        int result = psearch_leaf(next);
        if (result <= alpha) {
          return result;
        }
        next = state::put_black_at_rev(rev_bd, pos2);
        if (next.black() == black) {
          return result;
        }
        return std::min(result, psearch_leaf(next));
      }
    }
    return -psearch_leaf(next);
  } else {
    int result = -psearch_leaf(next);
    if (result >= beta) {
      return result;
    }
    next = state::put_black_at_rev(bd, pos2);
    if (next.black() == white) {
      return result;
    }
    return std::max(result, -psearch_leaf(next));
  }
}

int GameSolver::psearch_impl(const board &bd, int alpha, int beta, int ybwc_depth) {
  int stones = bit_manipulations::stone_sum(bd);
  if (stones <= 50 && ybwc_depth) {
    return psearch_ybwc(bd, alpha, beta, ybwc_depth);
  } else {
    return psearch_ordering(bd, alpha, beta, ybwc_depth);
  }
}

int GameSolver::psearch_nohash(const board &bd, int alpha, int beta) {
  ++nodes;
  int stones = bit_manipulations::stone_sum(bd);
  if (stones <= 57) {
    return psearch_static_ordering(bd, alpha, beta);
  } else if (stones <= 61) {
    return psearch_noordering(bd, alpha, beta);
  } else {
    return psearch_2(bd, alpha, beta);
  }
}

int GameSolver::psearch(const board &bd, int alpha, int beta, int ybwc_depth) {
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
        auto res = psearch_impl(bd, alpha, beta, ybwc_depth);
        tb[0].update(bd, new_ab, res);
        return res;
      } 
    } else {
      auto res = psearch_impl(bd, alpha, beta, ybwc_depth);
      tb[0].update(bd, table::Range(alpha, beta), res);
      return res;
    }
  } else {
    return psearch_nohash(bd, alpha, beta);
  }
}
