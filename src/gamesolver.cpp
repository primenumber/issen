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
constexpr int ONE_PLY = 64;
int reduction_table[2][8] = {
  {64, 72, 80, 88, 96,104,112,120},
  {32, 64, 64, 64, 64, 64, 64, 64}
};

template <bool is_PV>
int reduction(int move_count, bool enable_variable_reduction) {
  return enable_variable_reduction
    ? reduction_table[is_PV][std::min(move_count, 7)]
    : ONE_PLY;
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

std::atomic<uint64_t> nodes(0);
GameSolver::GameSolver(size_t hash_size)
    : tb{table::Table(hash_size), table::Table(hash_size)} {}

std::tuple<hand, int> GameSolver::think(const board &bd, const GameSolverParam solver_param, int depth_max) {
  param = solver_param;
  nodes = 0;
  rem_stones = 64 - bit_manipulations::stone_sum(bd);
  for (int depth = 5 * ONE_PLY; depth <= (depth_max - 1) * ONE_PLY; depth += ONE_PLY) {
    tb[0].clear();
    if (param.debug) std::cerr << "depth: " << (depth/ONE_PLY) << std::endl;
    int res = iddfs<true>(bd, -value::VALUE_MAX, value::VALUE_MAX, depth);
    if (param.debug) std::cerr << res << std::endl;
    std::swap(tb[0], tb[1]);
  }
  std::tuple<int, hand> mx(-value::VALUE_MAX-1, PASS);
  for (const auto &next : state::next_states(bd)) {
    hand h = hand_from_diff(bd, next);
    int res = -iddfs<true>(next, -value::VALUE_MAX, value::VALUE_MAX, (depth_max - 1) * ONE_PLY);
    mx = std::max(mx, std::make_tuple(res, h));
  }
  return std::make_tuple(std::get<1>(mx), std::get<0>(mx)/100);
}

int GameSolver::solve(const board &bd, const GameSolverParam solver_param) {
  param = solver_param;
  nodes = 0;
  rem_stones = 64 - bit_manipulations::stone_sum(bd);
  int res = 0;
  for (int depth = std::min(rem_stones * reduction<true>(0, param.enable_variable_reduction), ONE_PLY * 10); depth <= (rem_stones - 8) * ONE_PLY; depth += ONE_PLY) {
    tb[0].clear();
    if (param.debug) std::cerr << "depth: " << (depth/ONE_PLY) << std::endl;
    res = iddfs<true>(bd, -value::VALUE_MAX, value::VALUE_MAX, depth);
    if (param.debug) std::cerr << res << std::endl;
    std::swap(tb[0], tb[1]);
  }
  if (!param.perfect) {
    return res / 100;
  }
  tb[0].range_max = 64;
  tb[0].clear();
  if (param.debug) std::cerr << "full search" << std::endl;
  res = psearch(bd, -64, 64, param.parallel_search ? YBWC_Type::Type1 : YBWC_Type::NoYBWC);
  if (param.debug) {
    std::cerr << "nodes total: " << nodes.load() << std::endl;
    std::cerr << "hash update: " << (tb[0].update_num() + tb[1].update_num()) << std::endl;
    std::cerr << "hash conflict: " << (tb[0].conflict_num() + tb[1].conflict_num()) << std::endl;
  }
  return res;
}

template <typename InputIt, bool is_PV>
bool GameSolver::iddfs_ordering_impl(
    InputIt next_first, InputIt next_last,
    int &alpha, int beta, int &result,
    int depth, int &count) {
  using T = typename std::iterator_traits<InputIt>::value_type;
  std::sort(next_first, next_last, (bool(*)(const T&, const T&))order_impl);
  auto itr = next_first;
  if (itr == next_last) return false;
  if (count == 0) {
    const auto &next = *itr;
    result = std::max(result, -iddfs<is_PV>(get_board(next), -beta, -alpha, depth-reduction<is_PV>(0, param.enable_variable_reduction)));
    ++count;
    if (result >= beta) return true;
    alpha = std::max(alpha, result);
    ++itr;
  }
  for (; itr != next_last; ++itr, ++count) {
    const auto &next = *itr;
    result = std::max(result, -iddfs<false>(get_board(next), -alpha-1, -alpha, depth-reduction<false>(count, param.enable_variable_reduction)));
    if (result >= beta) return true;
    if (result <= alpha) continue;
    alpha = result;
    result = std::max(result, -iddfs<false>(get_board(next), -beta, -alpha, depth-reduction<false>(count, param.enable_variable_reduction)));
    if (result >= beta) return true;
    alpha = std::max(alpha, result);
  }
  return false;
}

template <bool is_PV> int GameSolver::iddfs_ordering(
    const board &bd, int alpha, int beta, int depth) {
  int stone_sum = bit_manipulations::stone_sum(bd);
  std::array<board, 60> next_buffer;
  int puttable_count = state::next_states(bd, next_buffer);
  if (puttable_count == 0) {
    board rev_bd = board::reverse_board(bd);
    if (state::puttable_black(rev_bd) == 0) {
      return value::num_value(bd);
    } else {
      return -iddfs<is_PV>(rev_bd, -beta, -alpha, depth);
    }
  }
  std::array<std::tuple<int, int, board>, 60> in_hash;
  std::array<std::tuple<int, board>, 60> out_hash;
  int count_in = 0;
  int count_out = 0;
  if (stone_sum > 54) {
    for (int i = 0; i < puttable_count; ++i) {
      const auto &next = next_buffer[i];
      out_hash[count_out++] = std::make_tuple(state::puttable_black_count(next), next);
    }
  } else {
    for (int i = 0; i < puttable_count; ++i) {
      const auto &next = next_buffer[i];
      if (auto val_opt = tb[1][next]) {
        in_hash[count_in++] = std::make_tuple(val_opt->val_max, val_opt->val_min, next);
      } else {
        out_hash[count_out++] = std::make_tuple(state::puttable_black_count(next), next);
      }
    }
  }
  int result = -value::VALUE_MAX; // fail soft
  int count = 0;
  if (iddfs_ordering_impl<decltype(std::begin(in_hash)), is_PV>(std::begin(in_hash), std::begin(in_hash) + count_in,
        alpha, beta, result, depth, count))
    return result;
  alpha = std::max(alpha, result);
  iddfs_ordering_impl<decltype(std::begin(out_hash)), is_PV>(std::begin(out_hash), std::begin(out_hash) + count_out,
      alpha, beta, result, depth, count);
  return result;
}

template <bool is_PV> int GameSolver::iddfs_impl(
    const board &bd, int alpha, int beta, int depth) {
  int stones = bit_manipulations::stone_sum(bd);
  if (stones < 60) {
    return iddfs_ordering<is_PV>(bd, alpha, beta, depth);
  } else {
    return psearch_impl(bd, alpha, beta, YBWC_Type::NoYBWC) * 100;
  }
}

template <bool is_PV> int GameSolver::iddfs(
    const board &bd, int alpha, int beta, int depth) {
  ++nodes;
  if (depth <= 0) {
    return value::statistic_value(bd);
  }
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
      auto res = iddfs_impl<is_PV>(bd, alpha, beta, depth);
      tb[0].update(bd, new_ab, res);
      return res;
    } 
  } else {
    auto res = iddfs_impl<is_PV>(bd, alpha, beta, depth);
    tb[0].update(bd, table::Range(alpha, beta), res);
    return res;
  }
}

template <typename InputIt>
bool GameSolver::psearch_ordering_impl(
    InputIt next_first, InputIt next_last,
    int &alpha, int beta, int &result, bool &first, const YBWC_Type type) {
  using T = typename std::iterator_traits<InputIt>::value_type;
  std::sort(next_first, next_last, (bool(*)(const T&, const T&))order_impl);
  for (auto itr = next_first; itr != next_last; ++itr) {
    const auto &next = *itr;
    if (!first) {
      result = std::max(result,
          -psearch(get_board(next), -alpha-1, -alpha, type));
      if (result >= beta) return true;
      if (result <= alpha) continue;
      alpha = result;
    } else {
      first = false;
    }
    result = std::max(result,
        -psearch(get_board(next), -beta, -alpha, type));
    if (result >= beta) {
      return true;
    }
    alpha = std::max(alpha, result);
  }
  return false;
}

int GameSolver::null_window_search_impl(const board &bd, int alpha, int beta, const YBWC_Type type) {
  int result = -psearch(bd, -alpha-1, -alpha, type);
  if (result >= beta) return result;
  if (result >= alpha) {
    result = -psearch(bd, -beta, -result, type);
  }
  return result;
}

int GameSolver::psearch_ybwc(const board &bd, int alpha, int beta, const YBWC_Type type) {
  std::array<board, 60> next_buffer;
  int puttable_count = state::next_states(bd, next_buffer);
  if (puttable_count == 0) {
    board rev_bd = board::reverse_board(bd);
    if (state::puttable_black(rev_bd) == 0) {
      return value::fixed_diff_num(bd);
    } else {
      return -psearch(rev_bd, -beta, -alpha, eldest_child(type));
    }
  }
  std::array<std::tuple<int, int, int, board>, 60> nexts;
  int count_in = 0;
  for (int i = 0; i < puttable_count; ++i) {
    const auto &next = next_buffer[i];
    int pcnt = state::puttable_black_count(next);
    if (auto val_opt = tb[1][next]) {
      nexts[i] = std::make_tuple(val_opt->val_max, val_opt->val_min, pcnt, next);
      ++count_in;
    } else {
      nexts[i] = std::make_tuple(value::VALUE_MAX, value::VALUE_MAX, pcnt, next);
    }
  }
  sort(std::begin(nexts), std::begin(nexts) + puttable_count);
  int seq_count;
  switch (type) {
    case YBWC_Type::Type1:
      seq_count = 1;
      break;
    case YBWC_Type::Type2:
      seq_count = std::max(1, count_in);
      break;
    case YBWC_Type::Type3:
      seq_count = 0;
      break;
    default:
      seq_count = puttable_count;
  }
  int result = -64; // fail soft
  for (int i = 0; i < seq_count; ++i) {
    result = std::max(result, -psearch(std::get<3>(nexts[i]), -beta, -alpha, i ? uneldest_child(type) : eldest_child(type)));
    if (result >= beta) return result;
    alpha = std::max(alpha, result);
  }
  std::vector<std::future<int>> vf;
  for (int i = seq_count; i < puttable_count; ++i) {
    vf.push_back(std::async(
          std::launch::async, &GameSolver::null_window_search_impl, this, std::get<3>(nexts[i]), alpha, beta,
          (i ? uneldest_child(type) : eldest_child(type)) ));
  }
  for (auto &&f : vf) {
    result = std::max(result, f.get());
  }
  return result;
}

int GameSolver::psearch_ordering(const board &bd, int alpha, int beta, const YBWC_Type type) {
  std::array<board, 60> next_buffer;
  int puttable_count = state::next_states(bd, next_buffer);
  if (puttable_count == 0) {
    board rev_bd = board::reverse_board(bd);
    if (state::puttable_black(rev_bd) == 0) {
      return value::fixed_diff_num(bd);
    } else {
      return -psearch(rev_bd, -beta, -alpha, eldest_child(type));
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
      out_hash[count_out++] = std::make_tuple(state::puttable_black_count(next), next);
    }
  }
  int result = -64; // fail soft
  bool first = true;
  if (psearch_ordering_impl(std::begin(in_hash), std::begin(in_hash) + count_in,
        alpha, beta, result, first, type))
    return result;
  alpha = std::max(alpha, result);
  psearch_ordering_impl(std::begin(out_hash), std::begin(out_hash) + count_out,
      alpha, beta, result, first, type);
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

int GameSolver::psearch_impl(const board &bd, int alpha, int beta, const YBWC_Type type) {
  int stones = bit_manipulations::stone_sum(bd);
  if (64 - stones >= rem_stones - 2 && type != YBWC_Type::NoYBWC) {
    return psearch_ybwc(bd, alpha, beta, type);
  } else {
    return psearch_ordering(bd, alpha, beta, type);
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

int GameSolver::psearch(const board &bd, int alpha, int beta, const YBWC_Type type) {
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
        auto res = psearch_impl(bd, alpha, beta, type);
        tb[0].update(bd, new_ab, res);
        return res;
      } 
    } else {
      auto res = psearch_impl(bd, alpha, beta, type);
      tb[0].update(bd, table::Range(alpha, beta), res);
      return res;
    }
  } else {
    return psearch_nohash(bd, alpha, beta);
  }
}
