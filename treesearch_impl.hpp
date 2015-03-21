template <typename Func>
int dfs(const board &bd, int depth,
    int alpha, int beta, const Func &func) {
  if (depth == 0) return func(bd);
  for (const auto &nx : state::next_states(bd)) {
    alpha = std::max(alpha,
        -dfs(nx, depth - 1, -beta, -alpha, func));
    if (alpha >= beta) return alpha;
  }
  return alpha;
}


template <typename Func>
int tree_negaalpha(node &nd, int depth,
    int alpha, int beta, const Func &func) {
  if (depth == 0) return func(nd.bd);
  if (nd.children.empty()) {
    return dfs(nd.bd, depth, alpha, beta, func);
  } else {
    for (auto &child : nd.children) {
      alpha = std::max(alpha,
          -tree_negaalpha(*child, depth - 1, -beta, -alpha, func));
      if (alpha >= beta) return alpha;
    }
    return alpha;
  }
}
