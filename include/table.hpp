#pragma once
#include <functional>
#include <tuple>
#include <vector>
#include <mutex>
#include <memory>

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

union RangeUint64 {
  Range range;
  uint64_t data;
  RangeUint64(const Range &r) : range(r) {}
  RangeUint64(const uint64_t d) : data(d) {}
};

class Entry {
 public:
  Entry() : data(_mm256_setzero_si256()) {}
  Entry(board bd, Range range) : data(_mm256_castsi128_si256(bd)) {
    RangeUint64 ru = range;
    data = _mm256_insert_epi64(data, ru.data, 2);
  }
  explicit Entry(int32_t range_max)
    : data(_mm256_setr_epi64x(0, 0, RangeUint64(Range(range_max)).data, 0)) {}
  Entry(const Entry &) = default;
  Entry(const __m256i &data) : data(data) {}
  Entry& operator=(const Entry &) noexcept = default;
  Entry& operator=(Entry &&) noexcept = default;
  operator __m256i() const { return data; }
  board get_board() const { return _mm256_castsi256_si128(data); }
  void set_board(const board &bd) { data = _mm256_inserti128_si256(data, bd, 0); }
  Range get_range() const {
    return RangeUint64(_mm256_extract_epi64(data, 2)).range;
  }
  void set_range(const Range &r) {
    data = _mm256_insert_epi64(data, RangeUint64(r).data, 2);
  }
 private:
  __m256i data;
};

class EntryArray {
 public:
  EntryArray(size_t array_size, Entry e) 
    : array((Entry*)aligned_alloc(32, array_size * 32), free), array_size(array_size) {
    std::fill(array.get(), array.get() + array_size, e);
  }
  EntryArray(EntryArray &&) = default;
  EntryArray &operator=(EntryArray &&) = default;
  Entry load(size_t index) const {
    return _mm256_load_si256((__m256i*)array.get() + index);
  }
  void store(size_t index, Entry e) {
    _mm256_store_si256((__m256i*)array.get() + index, e);
  }
  void clear(int32_t range_max) {
    std::fill(array.get(), array.get() + array_size, Entry(range_max));
  }
 private:
  std::unique_ptr<Entry, decltype(&free)> array;
  size_t array_size;
};

class Table {
 public:
  explicit Table(size_t hash_size)
    : table(hash_size, Entry(range_max)),
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
  EntryArray table;
  size_t hash_size;
  std::hash<board> bd_hash;
 public:
  int32_t range_max = value::VALUE_MAX;
  uint64_t conflict_count;
  uint64_t update_count;
};

} // namespace table
