#include "table.hpp"

#include <algorithm>

namespace table {

boost::optional<Range> Table::operator[](const board &bd) const {
  uint64_t h = bd_hash(bd);
  const auto &p = table[h % hash_size];
  if (p.first == bd) {
    return p.second;
  } else {
    return boost::none;
  }
}

uint64_t cnt = 0;
void Table::update(
    const board &bd, const Range range, const int32_t value) {
  ++cnt;
  uint64_t h = bd_hash(bd);
  auto &p = table[h % hash_size];
  if (range.val_min < value && value < range.val_max) {
    p = std::make_pair(bd, Range(value));
  } else {
    if (p.first == bd) {
      if (value >= range.val_max) {
        p.second.update_min(value);
      } else if (value <= range.val_min) {
        p.second.update_max(value);
      }
    } else {
      Range r(-range_max, range_max);
      if (value >= range.val_max) {
        r.update_min(value);
      } else if (value <= range.val_min) {
        r.update_max(value);
      }
      p = std::make_pair(bd, r);
    }
  }
}

void Table::clear() {
  std::fill(std::begin(table), std::end(table),
      std::make_pair(board::initial_board(), Range(-range_max, range_max)));
}

} // namespace table
