#include "tree.hpp"

#include "state.hpp"

namespace tree {

int expand_recursive(node &nd, int depth) {
  if (depth == 0) return 1;
  if (nd.children.empty())
    for (auto nx : state::next_states(nd.bd))
      nd.children.push_back(std::make_unique<node>(nx));
  int res = 0;
  for (auto &child : nd.children)
    res += expand_recursive(*child, depth - 1);
  return res;
}

} // tree
