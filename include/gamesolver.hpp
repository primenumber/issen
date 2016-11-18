#pragma once
#include "board.hpp"
#include "table.hpp"

class GameSolver {
 public:
  explicit GameSolver(size_t hash_size);
  int iddfs(const board &, bool parallel_search = true, bool debug = false);
 private:
  table::Table tb[2];
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
