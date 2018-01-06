#include "table.hpp"

#include <algorithm>

#include "x86intrin.h"

namespace table {

using unique_lock = std::unique_lock<std::mutex>;

boost::optional<std::tuple<Range, hand>> Table::operator[](const board &bd) const {
  uint64_t h = bd_hash(bd);
  std::size_t index = h % hash_size;
  Entry e = table.load(index);
  if (e.get_board() == bd) {
    return std::make_tuple(e.get_range(), e.get_pv());
  } else {
    return boost::none;
  }
}

void Table::update(
    const board &bd, const Range range, const Result result) {
  const int32_t value = result.value;
  ++update_count;
  uint64_t h = bd_hash(bd);
  std::size_t index = h % hash_size;
  Entry e = table.load(index);
  if (range.val_min < value && value < range.val_max) {
    e = Entry(bd, Range(value), result.h);
  } else {
    if (e.get_board() == bd) {
      Range r = e.get_range();
      if (value >= range.val_max) {
        r.update_min(value);
      } else if (value <= range.val_min) {
        r.update_max(value);
      }
      e.set_range(r);
      e.set_pv(result.h);
    } else {
      if (!(e.get_board() == board::empty_board())) {
        ++conflict_count;
        if (bit_manipulations::stone_sum(e.get_board()) < bit_manipulations::stone_sum(bd)) return;
      }
      Range r(-range_max, range_max);
      if (value >= range.val_max) {
        r.update_min(value);
      } else if (value <= range.val_min) {
        r.update_max(value);
      }
      e = Entry(bd, r, result.h);
    }
  }
  table.store(index, e);
}

void Table::clear() {
  table.clear(range_max);
}

} // namespace table
