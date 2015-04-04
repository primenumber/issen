#include "tree.hpp"

#include "state.hpp"

namespace tree {

void expand_recursive(node &nd, int depth) {
  if (depth == 0) return;
  if (nd.children.empty())
    for (auto nx : state::next_states(nd.bd))
      nd.children.push_back(std::make_unique<node>(nx));
  for (auto &child : nd.children)
    expand_recursive(*child, depth - 1);
}

} // tree
