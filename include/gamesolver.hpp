#pragma once
#include "board.hpp"
#include "table.hpp"

class GameSolver {
 public:
  explicit GameSolver(size_t hash_size);
  int iddfs(const board &);
 private:
  table::Table tb[2];
  std::vector<std::vector<board>> next_buffer;
  std::vector<std::vector<std::pair<int, board>>> in_buffer, out_buffer;
  int iddfs(const board &bd, int alpha, int beta, int depth, bool is_pn);
  bool iddfs_ordering_impl(std::vector<std::pair<int, board>> &ary, int &alpha, int beta, int &result, int depth, bool is_pn, bool &first);
  int iddfs_ordering(const board &bd, int alpha, int beta, int depth, bool is_pn);
  int iddfs_impl(const board &bd, int alpha, int beta, int depth, bool is_pn);
  int psearch(const board &bd, int alpha, int beta);
  int psearch_impl(const board &bd, int alpha, int beta);
  int psearch_ordering(const board &bd, int alpha, int beta);
  bool psearch_ordering_impl(std::vector<std::pair<int, board>> &ary, int &alpha, int beta, int &result, bool &first);
  int psearch_noordering(const board &bd, int alpha, int beta);
  int psearch_leaf(const board &bd);
  int psearch_2(const board &bd, int alpha, int beta);
};
