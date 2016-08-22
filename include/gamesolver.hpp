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
  int iddfs(const board &bd, int alpha, int beta);
  int iddfs_impl(const board &bd, int alpha, int beta);
  int iddfs_ordering(const board &bd, int alpha, int beta);
  int iddfs_noordering(const board &bd, int alpha, int beta);
  int iddfs_noordering2(const board &bd, int alpha, int beta);
  int iddfs_leaf(const board &bd);
  int rem_stones;
};
