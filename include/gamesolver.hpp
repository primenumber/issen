#pragma once
#include "board.hpp"
#include "table.hpp"

class GameSolver {
 public:
  explicit GameSolver(size_t hash_size)
    : tb{table::Table(hash_size), table::Table(hash_size)} {}
  int iddfs(const board &);
 private:
  table::Table tb[2];
  int dfs(const board &bd, int alpha, int beta);
  int dfs_impl(const board &bd, int alpha, int beta);
  int dfs_ordering(const board &bd, int alpha, int beta);
  int dfs_noordering(const board &bd, int alpha, int beta);
  int dfs_noordering2(const board &bd, int alpha, int beta);
  int dfs_leaf(const board &bd);
  int rem_stones;
};
