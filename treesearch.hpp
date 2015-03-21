#pragma once
#include <memory>
#include <vector>

#include "board.hpp"
#include "state.hpp"
#include "value.hpp"

namespace treesearch {

struct node {
  const board bd;
  std::vector<std::unique_ptr<node>> children;
  std::vector<int> value;
  node(const board &bd) : bd(bd) {}
};

void expand(node &, const board &);

} // namespace treesearch
