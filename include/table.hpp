#pragma once
#include <functional>
#include <utility>
#include <vector>

#include <boost/optional.hpp>

#include "board.hpp"
#include "bit_manipulations.hpp"
#include "value.hpp"

namespace table {

struct Range {
 public:
  int32_t val_min, val_max;
  Range(int32_t val_min, int32_t val_max) 
    : val_min(val_min), val_max(val_max) {}
  explicit Range(int32_t val)
    : val_min(val), val_max(val) {}
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
    : table(hash_size, std::make_pair(board::empty_board(), Range(-value::VALUE_MAX, value::VALUE_MAX))),
      hash_size(hash_size), bd_hash(), conflict_count(0), update_count(0) {}
  Table(const Table &) = default;
  Table(Table &&) = default;
  Table &operator=(const Table &) = default;
  Table &operator=(Table &&) = default;
  boost::optional<Range> operator[](const board &bd) const;
  void update(const board &bd, const Range range, const int32_t value);
  void clear();
  uint64_t conflict_num() const { return conflict_count; }
  uint64_t update_num() const { return update_count; }
  void reset_count() {
    conflict_count = update_count = 0;
  }
 private:
  std::vector<std::pair<board, Range>> table;
  size_t hash_size;
  std::hash<board> bd_hash;
 public:
  int32_t range_max = value::VALUE_MAX;
  uint64_t conflict_count;
  uint64_t update_count;
};

} // namespace table
