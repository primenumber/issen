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

Result GameSolver::think(const board &bd, const GameSolverParam solver_param, int depth_max) {
  param = solver_param;
  nodes = 0;
  rem_stones = 64 - bit_manipulations::stone_sum(bd);
  for (int depth = 5 * ONE_PLY; depth <= (depth_max - 1) * ONE_PLY; depth += ONE_PLY) {
    if (param.perfect) tb[0].range_max = value::VALUE_MAX;
    tb[0].clear();
    if (param.debug) std::cerr << "depth: " << (depth/ONE_PLY) << std::endl;
    int res = iddfs<true>(bd, -value::VALUE_MAX, value::VALUE_MAX, depth).value;
    if (param.debug) std::cerr << res << std::endl;
    std::swap(tb[0], tb[1]);
  }
  if (param.perfect) tb[0].range_max = 64;
  tb[0].clear();
  int alpha = param.perfect ? -64 : -value::VALUE_MAX;
  auto nexts = state::next_states(bd);
  hand mx = hand_from_diff(bd, nexts.front());
  for (const auto &next : nexts) {
    hand h = hand_from_diff(bd, next);
    int res;
    if (param.perfect) {
      res = -psearch<false>(next, -64, -alpha, YBWC_Type::NoYBWC).value;
    } else {
      res = -iddfs<true>(next, -value::VALUE_MAX, -alpha, (depth_max - 1) * ONE_PLY).value;
    }
    if (res > alpha) {
      alpha = res;
      mx = h;
    }
  }
  if (param.perfect)
    return Result(mx, alpha*100);
  else
    return Result(mx, alpha);
}

int GameSolver::solve(const board &bd, const GameSolverParam solver_param) {
  param = solver_param;
  nodes = 0;
  rem_stones = 64 - bit_manipulations::stone_sum(bd);
  int res = 0;
  for (int depth = std::min(rem_stones * reduction<true>(0, param.enable_variable_reduction), ONE_PLY * 10); depth <= (rem_stones - 8) * ONE_PLY; depth += ONE_PLY) {
    if (param.perfect) tb[0].range_max = value::VALUE_MAX;
    tb[0].clear();
    if (param.debug) std::cerr << "depth: " << (depth/ONE_PLY) << std::endl;
    res = iddfs<true>(bd, -value::VALUE_MAX, value::VALUE_MAX, depth).value;
    if (param.debug) std::cerr << res << std::endl;
    std::swap(tb[0], tb[1]);
  }
  if (!param.perfect) {
    return res / 100;
  }
  tb[0].range_max = 64;
  tb[0].clear();
  //if (param.debug) std::cerr << "pre search" << std::endl;
  //res = psearch<true>(bd, -64, 64, param.parallel_search ? YBWC_Type::Type1 : YBWC_Type::NoYBWC);
  //if (param.debug) std::cerr << res << std::endl;
  //std::swap(tb[0], tb[1]);
  //tb[0].clear();
  if (param.debug) std::cerr << "full search" << std::endl;
  res = psearch<false>(bd, -64, 64, param.parallel_search ? YBWC_Type::Type1 : YBWC_Type::NoYBWC).value;
  if (param.debug) {
    std::cerr << "nodes total: " << nodes.load() << std::endl;
    std::cerr << "hash update: " << (tb[0].update_num() + tb[1].update_num()) << std::endl;
    std::cerr << "hash conflict: " << (tb[0].conflict_num() + tb[1].conflict_num()) << std::endl;
  }
  return res;
}

template <typename InputIt, bool is_PV>
bool GameSolver::iddfs_ordering_impl(
    const board &bd, InputIt next_first, InputIt next_last,
    int &alpha, int beta, Result &result,
    int depth, int &count) {
  using T = typename std::iterator_traits<InputIt>::value_type;
  std::sort(next_first, next_last, (bool(*)(const T&, const T&))order_impl);
  auto itr = next_first;
  if (itr == next_last) return false;
  if (count == 0) {
    const auto &next = *itr;
    result = std::max(result, Result(hand_from_diff(bd, get_board(next)), -iddfs<is_PV>(get_board(next), -beta, -alpha, depth-reduction<is_PV>(0, param.enable_variable_reduction)).value));
    ++count;
    if (result.value >= beta) return true;
    alpha = std::max(alpha, result.value);
    ++itr;
  }
  for (; itr != next_last; ++itr, ++count) {
    const auto &next = *itr;
    result = std::max(result, Result(hand_from_diff(bd, get_board(next)), -iddfs<false>(get_board(next), -alpha-1, -alpha, depth-reduction<false>(count, param.enable_variable_reduction)).value));
    if (result.value >= beta) return true;
    if (result.value <= alpha) continue;
    alpha = result.value;
    result = std::max(result, Result(hand_from_diff(bd, get_board(next)), -iddfs<false>(get_board(next), -beta, -alpha, depth-reduction<false>(count, param.enable_variable_reduction)).value));
    if (result.value >= beta) return true;
    alpha = std::max(alpha, result.value);
  }
  return false;
}

template <bool is_PV> Result GameSolver::iddfs_ordering(
    const board &bd, int alpha, int beta, int depth) {
  int stone_sum = bit_manipulations::stone_sum(bd);
  std::array<board, 60> next_buffer;
  int puttable_count = state::next_states(bd, next_buffer);
  if (puttable_count == 0) {
    board rev_bd = board::reverse_board(bd);
    if (state::mobility_pos(rev_bd) == 0) {
      return Result(NOMOVE, value::num_value(bd));
    } else {
      return Result(PASS, -iddfs<is_PV>(rev_bd, -beta, -alpha, depth).value);
    }
  }
  std::array<std::tuple<int, int, board>, 60> in_hash;
  std::array<std::tuple<int, board>, 60> out_hash;
  int count_in = 0;
  int count_out = 0;
  if (stone_sum > 54) {
    for (int i = 0; i < puttable_count; ++i) {
      const auto &next = next_buffer[i];
      out_hash[count_out++] = std::make_tuple(state::mobility_count(next), next);
    }
  } else {
    for (int i = 0; i < puttable_count; ++i) {
      const auto &next = next_buffer[i];
      if (auto val_opt = tb[1][next]) {
        table::Range range = std::get<0>(*val_opt);
        in_hash[count_in++] = std::make_tuple(range.val_max, range.val_min, next);
      } else {
        out_hash[count_out++] = std::make_tuple(state::mobility_count(next), next);
      }
    }
  }
  Result result(NOMOVE, -value::VALUE_MAX); // fail soft
  int count = 0;
  if (iddfs_ordering_impl<decltype(std::begin(in_hash)), is_PV>(bd, std::begin(in_hash), std::begin(in_hash) + count_in,
        alpha, beta, result, depth, count))
    return result;
  alpha = std::max(alpha, result.value);
  iddfs_ordering_impl<decltype(std::begin(out_hash)), is_PV>(bd, std::begin(out_hash), std::begin(out_hash) + count_out,
      alpha, beta, result, depth, count);
  return result;
}

template <bool is_PV> Result GameSolver::iddfs_impl(
    const board &bd, int alpha, int beta, int depth) {
  int stones = bit_manipulations::stone_sum(bd);
  if (stones < 60) {
    return iddfs_ordering<is_PV>(bd, alpha, beta, depth);
  } else {
    Result res = psearch<false>(bd, -64, 64, YBWC_Type::NoYBWC);
    res.value *= 100;
    return res;
  }
}

template <bool is_PV> Result GameSolver::iddfs(
    const board &bd, int alpha, int beta, int depth) {
  ++nodes;
  if (depth <= 0) {
    return Result(NOMOVE, value::statistic_value(bd));
  }
  if (const auto cache_opt = tb[0][bd]) {
    table::Range cache_r(-value::VALUE_MAX, value::VALUE_MAX);
    hand cache_pv;
    std::tie(cache_r, cache_pv) = *cache_opt;
    if (cache_r.val_min >= beta) {
      return Result(cache_pv, cache_r.val_min);
    } else if (cache_r.val_max <= alpha) {
      return Result(cache_pv, cache_r.val_max);
    } else {
      table::Range new_ab = cache_r && table::Range(alpha, beta);
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

template <typename InputIt, bool probcut>
bool GameSolver::psearch_ordering_impl(
    const board &bd, InputIt next_first, InputIt next_last,
    int &alpha, int beta, Result &result, bool &first, const YBWC_Type type) {
  using T = typename std::iterator_traits<InputIt>::value_type;
  std::sort(next_first, next_last, (bool(*)(const T&, const T&))order_impl);
  for (auto itr = next_first; itr != next_last; ++itr) {
    const auto &next = *itr;
    if (!first) {
      if (probcut && -value::statistic_value(get_board(next)) + 1200 < alpha * 100) continue;
      result = std::max(result,
          Result(hand_from_diff(bd, get_board(next)), -psearch<probcut>(get_board(next), -alpha-1, -alpha, type).value));
      if (result.value >= beta) return true;
      if (result.value <= alpha) continue;
      alpha = result.value;
    } else {
      first = false;
    }
    result = std::max(result,
        Result(hand_from_diff(bd, get_board(next)), -psearch<probcut>(get_board(next), -beta, -alpha, type).value));
    if (result.value >= beta) {
      return true;
    }
    alpha = std::max(alpha, result.value);
  }
  return false;
}

template <bool probcut> Result GameSolver::null_window_search_impl(const hand h, const board &bd, int alpha, int beta, const YBWC_Type type) {
  auto result = Result(h, -psearch<probcut>(bd, -alpha-1, -alpha, type).value);
  if (result.value >= beta) return result;
  if (result.value >= alpha) {
    result = Result(h, -psearch<probcut>(bd, -beta, -result.value, type).value);
  }
  return result;
}

template <bool probcut> Result GameSolver::psearch_ybwc(const board &bd, int alpha, int beta, const YBWC_Type type) {
  std::array<board, 60> next_buffer;
  int puttable_count = state::next_states(bd, next_buffer);
  if (puttable_count == 0) {
    board rev_bd = board::reverse_board(bd);
    if (state::mobility_pos(rev_bd) == 0) {
      return Result(NOMOVE, value::fixed_diff_num(bd));
    } else {
      return Result(PASS, -psearch<probcut>(rev_bd, -beta, -alpha, eldest_child(type)).value);
    }
  }
  std::array<std::tuple<int, int, int, board>, 60> nexts;
  int count_in = 0;
  for (int i = 0; i < puttable_count; ++i) {
    const auto &next = next_buffer[i];
    int pcnt = state::mobility_count(next);
    if (auto val_opt = tb[1][next]) {
      table::Range range = std::get<0>(*val_opt);
      nexts[i] = std::make_tuple(range.val_max, range.val_min, pcnt, next);
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
  Result result(NOMOVE, -64); // fail soft
  for (int i = 0; i < seq_count; ++i) {
    const auto next = std::get<3>(nexts[i]);
    result = std::max(result, Result(hand_from_diff(bd, next), -psearch<probcut>(next, -beta, -alpha, i ? uneldest_child(type) : eldest_child(type)). value));
    if (result.value >= beta) return result;
    alpha = std::max(alpha, result.value);
  }
  std::vector<std::future<Result>> vf;
  for (int i = seq_count; i < puttable_count; ++i) {
    const auto next = std::get<3>(nexts[i]);
    vf.push_back(std::async(
          std::launch::async, &GameSolver::null_window_search_impl<probcut>, this, hand_from_diff(bd, next), next, alpha, beta,
          (i ? uneldest_child(type) : eldest_child(type)) ));
  }
  for (auto &&f : vf) {
    result = std::max(result, f.get());
  }
  return result;
}

template <bool probcut> Result GameSolver::psearch_ordering(const board &bd, int alpha, int beta, const YBWC_Type type) {
  std::array<board, 60> next_buffer;
  int puttable_count = state::next_states(bd, next_buffer);
  if (puttable_count == 0) {
    board rev_bd = board::reverse_board(bd);
    if (state::mobility_pos(rev_bd) == 0) {
      return Result(NOMOVE, value::fixed_diff_num(bd));
    } else {
      return Result(PASS, -psearch<probcut>(rev_bd, -beta, -alpha, eldest_child(type)).value);
    }
  }
  std::array<std::tuple<int, int, board>, 60> in_hash;
  std::array<std::tuple<int, board>, 60> out_hash;
  int count_in = 0;
  int count_out = 0;
  for (int i = 0; i < puttable_count; ++i) {
    const auto &next = next_buffer[i];
    if (auto val_opt = tb[1][next]) {
      table::Range range = std::get<0>(*val_opt);
      in_hash[count_in++] = std::make_tuple(range.val_max, range.val_min, next);
    } else {
      out_hash[count_out++] = std::make_tuple(state::mobility_count(next), next);
    }
  }
  Result result(NOMOVE, -64); // fail soft
  bool first = true;
  if (psearch_ordering_impl<decltype(in_hash)::iterator, probcut>(bd, std::begin(in_hash), std::begin(in_hash) + count_in,
        alpha, beta, result, first, type))
    return result;
  alpha = std::max(alpha, result.value);
  psearch_ordering_impl<decltype(out_hash)::iterator, probcut>(bd, std::begin(out_hash), std::begin(out_hash) + count_out,
      alpha, beta, result, first, type);
  return result;
}

template <bool probcut> Result GameSolver::psearch_static_ordering(const board &bd, int alpha, int beta) {
  std::array<board, 60> next_buffer;
  int puttable_count = state::next_states(bd, next_buffer);
  if (puttable_count == 0) {
    board rev_bd = board::reverse_board(bd);
    if (state::mobility_pos(rev_bd) == 0) {
      return Result(NOMOVE, value::fixed_diff_num(bd));
    } else {
      return Result(PASS, -psearch_nohash<probcut>(rev_bd, -beta, -alpha).value);
    }
  }
  std::array<std::tuple<int, int>, 60> index_ary;
  int count_ary = 0;
  for (int i = 0; i < puttable_count; i += 2) {
    const auto &next = next_buffer[i];
    const auto &next2 = next_buffer[i+1];
    auto counts = state::mobility_count(double_board(next, next2));
    index_ary[count_ary++] = std::make_tuple(counts[0], i);
    index_ary[count_ary++] = std::make_tuple(counts[1], i+1);
  }
  if ((puttable_count % 2) == 1) {
    int i = puttable_count - 1;
    const auto &next = next_buffer[i];
    index_ary[count_ary++] = std::make_tuple(state::mobility_count(next), i);
  }
  std::sort(std::begin(index_ary), std::begin(index_ary) + puttable_count,
      [](const auto &lhs, const auto &rhs) { return std::get<0>(lhs) < std::get<0>(rhs); });
  const auto pv = next_buffer[std::get<1>(index_ary[0])];
  Result result = Result(hand_from_diff(bd, pv), -psearch_nohash<probcut>(pv, -beta, -alpha).value);
  if (result.value >= beta) return result;
  alpha = std::max(alpha, result.value);
  for (int i = 1; i < puttable_count; ++i) {
    const auto next = next_buffer[std::get<1>(index_ary[i])];
    result = std::max(result, Result(hand_from_diff(bd, next), -psearch_nohash<probcut>(next, -alpha-1, -alpha).value));
    if (result.value >= beta) return result;
    if (result.value <= alpha) continue;
    alpha = result.value;
    result = std::max(result, Result(hand_from_diff(bd, next), -psearch_nohash<probcut>(next, -beta, -alpha).value));
    if (result.value >= beta) return result;
    alpha = std::max(alpha, result.value);
  }
  return result;
}

template <bool probcut> Result GameSolver::psearch_noordering(const board &bd, int alpha, int beta) {
  bool pass = true;
  Result result(NOMOVE, -64); // fail soft
  uint64_t puttable_bits = ~bit_manipulations::stones(bd);
  for (; puttable_bits; puttable_bits = _blsr_u64(puttable_bits)) {
    const uint64_t bit = _blsi_u64(puttable_bits);
    const uint8_t pos = bit_manipulations::bit_to_pos(bit);
    const board next = state::move(bd, pos);
    if (next.player() == bd.opponent()) continue;
    pass = false;
    result = std::max(result, Result(pos, -psearch_nohash<probcut>(next, -beta, -alpha).value));
    if (result.value >= beta) {
      return result;
    }
    alpha = std::max(alpha, result.value);
  }
  if (pass) {
    const board rev_bd = board::reverse_board(bd);
    if (state::mobility_pos(rev_bd) == 0) {
      return Result(NOMOVE, value::fixed_diff_num(bd));
    } else {
      return Result(PASS, -psearch_nohash<probcut>(rev_bd, -beta, -alpha).value);
    }
  }
  return result;
}

Result GameSolver::psearch_leaf(const board &bd) {
  ++nodes;
  uint64_t pos_bit = ~bit_manipulations::stones(bd);
  int pos = bit_manipulations::bit_to_pos(pos_bit);
  const board nx = state::move_rev(bd, pos);
  if (nx.opponent() == bd.opponent()) {
    const board nx2 = state::move_rev(board::reverse_board(bd), pos);
    if (nx2.opponent() == bd.player()) {
      return Result(NOMOVE, value::fixed_diff_num(bd));
    } else {
      ++nodes;
      return Result(PASS, -value::fixed_diff_num(nx2));
    }
  } else {
    return Result(pos, value::fixed_diff_num(nx));
  }
}

Result GameSolver::psearch_2(const board &bd, int alpha, int beta) {
  const uint64_t black = bd.player();
  const uint64_t white = bd.opponent();
  const uint64_t puttable_bits = ~bit_manipulations::stones(bd);
  const uint64_t bit1 = _blsi_u64(puttable_bits);
  const uint8_t pos1 = bit_manipulations::bit_to_pos(bit1);
  board next = state::move(bd, pos1);
  const uint64_t bit2 = _blsr_u64(puttable_bits);
  const uint8_t pos2 = bit_manipulations::bit_to_pos(bit2);
  if (next.player() == white) {
    next = state::move(bd, pos2);
    if (next.player() == white) { // PASS
      ++nodes;
      const board rev_bd = board::reverse_board(bd);
      next = state::move(rev_bd, pos1);
      if (next.player() == black) {
        next = state::move(rev_bd, pos2);
        if (next.player() == black) {
          return Result(NOMOVE, value::fixed_diff_num(bd));
        } else {
          return Result(PASS, psearch_leaf(next).value);
        }
      } else {
        Result result = Result(PASS, psearch_leaf(next).value);
        if (result.value <= alpha) {
          return result;
        }
        next = state::move(rev_bd, pos2);
        if (next.player() == black) {
          return result;
        }
        return std::min(result, Result(PASS, psearch_leaf(next).value));
      }
    }
    return Result(pos2, -psearch_leaf(next).value);
  } else {
    Result result = Result(pos1, -psearch_leaf(next).value);
    if (result.value >= beta) {
      return result;
    }
    next = state::move(bd, pos2);
    if (next.player() == white) {
      return result;
    }
    return std::max(result, Result(pos2, -psearch_leaf(next).value));
  }
}

template <bool probcut> Result GameSolver::psearch_impl(const board &bd, int alpha, int beta, const YBWC_Type type) {
  int stones = bit_manipulations::stone_sum(bd);
  if (64 - stones >= rem_stones - 2 && type != YBWC_Type::NoYBWC) {
    return psearch_ybwc<probcut>(bd, alpha, beta, type);
  } else {
    return psearch_ordering<probcut>(bd, alpha, beta, type);
  }
}

template <bool probcut> Result GameSolver::psearch_nohash(const board &bd, int alpha, int beta) {
  ++nodes;
  int stones = bit_manipulations::stone_sum(bd);
  if (stones <= 57) {
    return psearch_static_ordering<probcut>(bd, alpha, beta);
  } else if (stones <= 61) {
    return psearch_noordering<probcut>(bd, alpha, beta);
  } else {
    return psearch_2(bd, alpha, beta);
  }
}

template <bool probcut> Result GameSolver::psearch(const board &bd, int alpha, int beta, const YBWC_Type type) {
  ++nodes;
  int stones = bit_manipulations::stone_sum(bd);
  if (stones <= 54) {
    if (const auto cache_opt = tb[0][bd]) {
      table::Range cache_r(-64, 64);
      hand cache_pv;
      std::tie(cache_r, cache_pv) = *cache_opt;
      if (cache_r.val_min >= beta) {
        return Result(cache_pv, cache_r.val_min);
      } else if (cache_r.val_max <= alpha) {
        return Result(cache_pv, cache_r.val_max);
      } else {
        table::Range new_ab = cache_r && table::Range(alpha, beta);
        alpha = new_ab.val_min;
        beta = new_ab.val_max;
        auto res = psearch_impl<probcut>(bd, alpha, beta, type);
        tb[0].update(bd, new_ab, res);
        return res;
      } 
    } else {
      auto res = psearch_impl<probcut>(bd, alpha, beta, type);
      tb[0].update(bd, table::Range(alpha, beta), res);
      return res;
    }
  } else {
    return psearch_nohash<probcut>(bd, alpha, beta);
  }
}
