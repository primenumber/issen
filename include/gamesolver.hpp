#pragma once
#include <tuple>
#include <boost/optional.hpp>
#include "board.hpp"
#include "hand.hpp"
#include "table.hpp"
#include "result.hpp"

struct GameSolverParam {
  bool parallel_search;
  bool debug;
  bool perfect;
  bool enable_variable_reduction;
  bool pre_search;
};

constexpr GameSolverParam solve_perfect = {false, false, true, true, true};

enum class YBWC_Type {
  Type1,
  Type2,
  Type3,
  NoYBWC
};

inline YBWC_Type eldest_child(const YBWC_Type type) {
  switch (type) {
    case YBWC_Type::Type1:
      return YBWC_Type::Type1;
    case YBWC_Type::Type2:
      return YBWC_Type::Type3;
    case YBWC_Type::Type3:
      return YBWC_Type::Type2;
    case YBWC_Type::NoYBWC:
      return YBWC_Type::NoYBWC;
    default:
      throw std::logic_error("illigal enum value");
  }
}

inline YBWC_Type uneldest_child(const YBWC_Type type) {
  switch (type) {
    case YBWC_Type::Type1:
    case YBWC_Type::Type2:
    case YBWC_Type::Type3:
      return YBWC_Type::Type2;
    case YBWC_Type::NoYBWC:
      return YBWC_Type::NoYBWC;
    default:
      throw std::logic_error("illigal enum value");
  }
}

class GameSolver {
 public:
  explicit GameSolver(size_t hash_size);
  Result think(const board &, const GameSolverParam solver_param, int depth_max);
  int solve(const board &, const GameSolverParam solver_param);
 private:
  table::Table tb[2];
  GameSolverParam param;
  int rem_stones;
  template <bool is_PV> Result iddfs(const board &bd, int alpha, int beta, int depth);
  template <typename InputIt, bool is_PV>
  bool iddfs_ordering_impl(const board &bd, InputIt next_first, InputIt next_last,
      int &alpha, int beta, Result &result, int depth, int &count);
  template <bool is_PV> Result iddfs_ordering(const board &bd, int alpha, int beta, int depth);
  template <bool is_PV> Result iddfs_impl(const board &bd, int alpha, int beta, int depth);
  template <bool probcut> Result psearch(const board &bd, int alpha, int beta, const YBWC_Type type);
  template <bool probcut> Result psearch_nohash(const board &bd, int alpha, int beta);
  template <bool probcut> Result psearch_impl(const board &bd, int alpha, int beta, const YBWC_Type type);
  template <bool probcut> Result null_window_search_impl(const hand h, const board &bd, int alpha, int beta, const YBWC_Type type);
  template <bool probcut> Result psearch_ybwc(const board &bd, int alpha, int beta, const YBWC_Type type);
  template <typename InputIt, bool probcut>
  bool psearch_ordering_impl(const board &bd, InputIt next_first, InputIt next_last,
      int &alpha, int beta, Result &result, bool &first, const YBWC_Type type);
  template <bool probcut> Result psearch_ordering(const board &bd, int alpha, int beta, const YBWC_Type type);
  template <bool probcut> Result psearch_static_ordering(const board &bd, int alpha, int beta);
  template <bool probcut> Result psearch_noordering(const board &bd, int alpha, int beta);
  Result psearch_leaf(const board &bd);
  Result psearch_2(const board &bd, int alpha, int beta);
};
