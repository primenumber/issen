#pragma once

#include <algorithm>
#include <memory>
#include <vector>

#include "board.hpp"
#include "value.hpp"

namespace tree {

struct node {
  const board bd;
  std::vector<std::unique_ptr<node>> children;
  int reorder_val;
  node(const node &) = default;
  node(node &&) = default;
  explicit node(const board &bd) : bd(bd) {}
};

int expand_recursive(node &nd, int depth);

template <typename Func>
void reorder(node &nd, Func func) {
  if (nd.children.empty()) {
    nd.reorder_val = func(nd.bd);
  } else {
    using np = std::unique_ptr<node>;
    for (auto &cp : nd.children)
      reorder(*cp, func);
    std::sort(std::begin(nd.children), std::end(nd.children),
        [](const np &lhs, const np &rhs) {
          return lhs->reorder_val < rhs->reorder_val;
        });
    nd.reorder_val = -nd.children.front()->reorder_val;
  }
}

}
