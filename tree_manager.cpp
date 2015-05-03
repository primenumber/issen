#include "tree_manager.hpp"

#include <boost/timer.hpp>

#include "bit_manipulations.hpp"

namespace tree_manager {

void tree_manager::update_tree(const board &nx) {
  for (auto &child : nd_ptr->children) {
    if (child->bd == nx) {
      nd_ptr.reset(child.release());
      break;
    }
  }
}

void tree_manager::reorder_tree() {
  int rem_stones = 64 - bit_manipulations::stone_sum(bd);
  if (rem_stones - dep >= 12) {
    boost::timer t;
    for (dep_rec = dep; t.elapsed() < 0.5 && dep_rec <= dep + 4; ++dep_rec) {
      tree::reorder_recursive(*nd_ptr, value::value, dep_rec);
    }
  } else {
    tree::reorder(*nd_ptr, value::value);
  }
}

void tree_manager::expand_tree() {
  int rem_stones = 64 - bit_manipulations::stone_sum(bd);
  while (dep <= rem_stones / 2) {
    int ts = tree::expand_recursive(*nd_ptr, dep);
    if (ts > 1000000) break;
    ++dep;
  }
}

} // namespace tree_manager
