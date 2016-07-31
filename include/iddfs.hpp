#include "board.hpp"

#include <iostream>
#include <vector>
#include <functional>
#include <utility>
#include <boost/optional.hpp>

#include "value.hpp"

struct Range {
 public:
  int32_t val_min, val_max;
  Range(int32_t val_min, int32_t val_max) 
    : val_min(val_min), val_max(val_max) {}
  explicit Range(int32_t val)
    : val_min(val), val_max(val) {}
  Range()
    : val_min(-value::VALUE_MAX), val_max(value::VALUE_MAX) {}
  Range(const Range &) = default;
  Range(Range &&) = default;
  Range &operator=(const Range &) = default;
  Range &operator=(Range &&) = default;
  explicit operator bool() const {
    return val_min <= val_max;
  }
  bool is_in_range(const int32_t val) const {
    return val_min <= val && val <= val_max;
  }
  int32_t update_max(const int32_t val) {
    return val_max = std::min(val_max, val);
  }
  int32_t update_min(const int32_t val) {
    return val_min = std::max(val_min, val);
  }
};

inline Range operator&&(const Range &lhs, const Range &rhs) {
  return Range(std::max(lhs.val_min, rhs.val_min),
      std::min(lhs.val_max, rhs.val_max));
}

class Table {
 public:
  explicit Table(size_t hash_size)
    : table(hash_size), hash_size(hash_size), bd_hash() {}
  Table(const Table &) = default;
  Table(Table &&) = default;
  Table &operator=(const Table &) = default;
  Table &operator=(Table &&) = default;
  boost::optional<Range> operator[](const board &bd) const {
    uint64_t h = bd_hash(bd);
    if (table[h % hash_size].first == bd) {
      return table[h % hash_size].second;
    } else {
      return boost::none;
    }
  }
  void update(const board &bd, const Range range, const int32_t value);
  void clear() {
    for (auto &p : table) {
      p = std::make_pair(board::initial_board(), Range());
    }
  }
 private:
  std::vector<std::pair<board, Range>> table;
  size_t hash_size;
  std::hash<board> bd_hash;
};

class GameSolver {
 public:
  explicit GameSolver(size_t hash_size)
    : table{Table(hash_size), Table(hash_size)} {}
  int iddfs(const board &);
 private:
  Table table[2];
  int dfs(const board &bd, int depth, int alpha, int beta);
  int dfs_impl(const board &bd, int depth, int alpha, int beta);
  int dfs_ordering(const board &bd, int depth, int alpha, int beta);
  int dfs_noordering(const board &bd, int depth, int alpha, int beta);
  int dfs_noordering2(const board &bd, int depth, int alpha, int beta);
  int dfs_leaf(const board &bd);
  int max_depth;
};
