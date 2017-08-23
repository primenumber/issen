#pragma once
#include "board.hpp"
#include "table.hpp"

struct GameSolverParam {
  bool parallel_search;
  bool debug;
  bool perfect;
  int iddfs_pv_extension = 50;
};

constexpr GameSolverParam solve_perfect = {false, false, true, 50};

class GameSolver {
 public:
  explicit GameSolver(size_t hash_size);
  int solve(const board &, const GameSolverParam solver_param);
 private:
  table::Table tb[2];
  GameSolverParam param;
  int iddfs(const board &bd, int alpha, int beta, int depth, bool is_pn);
  template <typename InputIt>
  bool iddfs_ordering_impl(InputIt next_first, InputIt next_last,
      int &alpha, int beta, int &result, int depth, bool is_pn, bool &first);
  int iddfs_ordering(const board &bd, int alpha, int beta, int depth, bool is_pn);
  int iddfs_impl(const board &bd, int alpha, int beta, int depth, bool is_pn);
  int psearch(const board &bd, int alpha, int beta, int ybwc_depth);
  int psearch_nohash(const board &bd, int alpha, int beta);
  int psearch_impl(const board &bd, int alpha, int beta, int ybwc_depth);
  int null_window_search_impl(const board &bd, int alpha, int beta, int result, int ybwc_depth);
  int psearch_ybwc(const board &bd, int alpha, int beta, int ybwc_depth);
  template <typename InputIt>
  bool psearch_ordering_impl(InputIt next_first, InputIt next_last,
      int &alpha, int beta, int &result, bool &first, int ybwc_depth);
  int psearch_ordering(const board &bd, int alpha, int beta, int ybwc_depth);
  int psearch_static_ordering(const board &bd, int alpha, int beta);
  int psearch_noordering(const board &bd, int alpha, int beta);
  int psearch_leaf(const board &bd);
  int psearch_2(const board &bd, int alpha, int beta);
};
