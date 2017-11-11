#pragma once
#include <tuple>
#include "board.hpp"
#include "hand.hpp"
#include "table.hpp"


struct GameSolverParam {
  bool parallel_search;
  bool debug;
  bool perfect;
  bool enable_variable_reduction;
};

constexpr GameSolverParam solve_perfect = {false, false, true, true};

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
  std::tuple<hand, int> think(const board &, const GameSolverParam solver_param, int depth_max);
  int solve(const board &, const GameSolverParam solver_param);
 private:
  table::Table tb[2];
  GameSolverParam param;
  int rem_stones;
  template <bool is_PV> int iddfs(const board &bd, int alpha, int beta, int depth);
  template <typename InputIt, bool is_PV>
  bool iddfs_ordering_impl(InputIt next_first, InputIt next_last,
      int &alpha, int beta, int &result, int depth, int &count);
  template <bool is_PV> int iddfs_ordering(const board &bd, int alpha, int beta, int depth);
  template <bool is_PV> int iddfs_impl(const board &bd, int alpha, int beta, int depth);
  template <bool probcut> int psearch(const board &bd, int alpha, int beta, const YBWC_Type type);
  template <bool probcut> int psearch_nohash(const board &bd, int alpha, int beta);
  template <bool probcut> int psearch_impl(const board &bd, int alpha, int beta, const YBWC_Type type);
  template <bool probcut> int null_window_search_impl(const board &bd, int alpha, int beta, const YBWC_Type type);
  template <bool probcut> int psearch_ybwc(const board &bd, int alpha, int beta, const YBWC_Type type);
  template <typename InputIt, bool probcut>
  bool psearch_ordering_impl(InputIt next_first, InputIt next_last,
      int &alpha, int beta, int &result, bool &first, const YBWC_Type type);
  template <bool probcut> int psearch_ordering(const board &bd, int alpha, int beta, const YBWC_Type type);
  template <bool probcut> int psearch_static_ordering(const board &bd, int alpha, int beta);
  template <bool probcut> int psearch_noordering(const board &bd, int alpha, int beta);
  int psearch_leaf(const board &bd);
  int psearch_2(const board &bd, int alpha, int beta);
};
