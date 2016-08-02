#include "tree.hpp"

#include "state.hpp"
#include "bit_manipulations.hpp"

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

void leafs_impl(const node &nd, std::vector<board> &vb) {
  if (nd.children.empty()) {
    vb.push_back(nd.bd);
  } else {
    for (auto &np : nd.children)
      leafs_impl(*np, vb);
  }
}

std::vector<board> leafs(const node &nd) {
  std::vector<board> vb;
  leafs_impl(nd, vb);
  return vb;
}

std::vector<board> unique_leafs(const node &nd) {
  auto vb = leafs(nd);
  std::sort(std::begin(vb), std::end(vb));
  vb.erase(std::unique(std::begin(vb), std::end(vb)),
      std::end(vb));
  return vb;
}

} // tree
