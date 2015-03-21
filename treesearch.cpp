#include <algorithm>

#include "treesearch.hpp"

namespace treesearch {

void expand(node &nd) {
  for (auto nx : state::next_states(nd.bd)) {
    auto child = std::make_unique<node>(nx);
    child->value.push_back(value::value(nx));
    nd.children.push_back(std::move(child));
  }
  std::sort(begin(nd.children), end(nd.children),
      [](const std::unique_ptr<node> &lhs,
          const std::unique_ptr<node> &rhs) {
        return lhs->value.back() < rhs->value.back();
      });
  nd.value.push_back(-nd.children.front()->value.back());
}

void expand_recursive(node &nd, int depth) {
  if (depth == 0) return;
  if (nd.children.empty()) {
    expand(nd);
  }
  for (auto &child : nd.children) {
    expand_recursive(*child, depth - 1);
  }
  int old_dep = nd.value.size();
  int new_dep = nd.children.front()->value.size() + 1;
  nd.value.resize(new_dep, -value::VALUE_MAX);
  for (const auto &child : nd.children) {
    for (int i = old_dep; i < new_dep; ++i) {
      nd.value[i] = std::max(nd.value[i], -child->value[i-1]);
    }
  }
  std::sort(begin(nd.children), end(nd.children),
      [](const std::unique_ptr<node> &lhs,
          const std::unique_ptr<node> &rhs) {
        return lhs->value.back() < rhs->value.back();
      });
}

int dfs(const board &bd, int depth, int alpha, int beta) {
  if (depth == 0) return value::value(bd);
  for (const auto &bd : state::next_states(bd)) {
    alpha = std::max(alpha,
        -dfs(bd, depth - 1, -beta, -alpha));
    if (alpha >= beta) return alpha;
  }
  return alpha;
}

int tree_negaalpha(node &nd, int depth, int alpha, int beta) {
  if (depth == 0) return value::value(nd.bd);
  if (nd.children.empty()) {
    return dfs(nd.bd, depth, alpha, beta);
  } else {
    for (auto &child : nd.children) {
      alpha = std::max(alpha,
          -tree_negaalpha(*child, depth - 1, -beta, -alpha));
      if (alpha >= beta) return alpha;
    }
    return alpha;
  }
}

int tree_negamax(node &nd, int depth) {
  return tree_negaalpha(nd, depth, -value::VALUE_MAX, value::VALUE_MAX);
}

} // namespace treesearch
