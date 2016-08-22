#include "table.hpp"

#include <algorithm>

namespace table {

boost::optional<Range> Table::operator[](const board &bd) const {
  uint64_t h = bd_hash(bd);
  if (table[h % hash_size].first == bd) {
    return table[h % hash_size].second;
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
        p.second.update_min(range.val_max);
      } else if (value <= range.val_min) {
        p.second.update_max(range.val_min);
      }
    } else {
      Range r;
      if (value >= range.val_max) {
        r.update_min(range.val_max);
      } else if (value <= range.val_min) {
        r.update_max(range.val_min);
      }
      p = std::make_pair(bd, r);
    }
  }
}

void Table::clear() {
  std::fill(std::begin(table), std::end(table),
      std::make_pair(board::initial_board(), Range()));
}

} // namespace table
