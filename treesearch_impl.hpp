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
  if (depth == 0) return nd.value.front() = func(nd.bd);
  if (depth >= nd.value.size())
    nd.value.resize(depth + 1, -value::VALUE_MAX);
  if (nd.children.empty()) {
    return nd.value[depth] = dfs(nd.bd, depth, alpha, beta, func);
  } else {
    for (auto &child : nd.children) {
      alpha = std::max(alpha,
          -tree_negaalpha(*child, depth - 1, -beta, -alpha, func));
      if (alpha >= beta) return nd.value[depth] = alpha;
    }
    return nd.value[depth] = alpha;
  }
}

template <typename Func>
std::pair<int, board> dfs_unlimited(const board &bd,
    int alpha, int beta, const Func &func) {
  if (state::puttable_black(bd) == 0 &&
      state::puttable_black(board::reverse_board(bd)) == 0)
    return std::make_pair(func(bd), bd);
  board opt;
  for (const auto &nx : state::next_states(bd)) {
    int val = -dfs_unlimited(nx, -beta, -alpha, func).first;
    if (val > alpha) {
      alpha = val;
      opt = nx;
    }
    if (alpha >= beta) return std::make_pair(alpha, bd);
  }
  return std::make_pair(alpha, opt);
}

template <typename Func>
std::pair<int, board> tree_negaalpha_unlimited(node &nd,
    int alpha, int beta, const Func &func) {
  if (nd.children.empty()) {
    return dfs_unlimited(nd.bd, alpha, beta, func);
  } else {
    board opt = nd.children.front()->bd;
    for (auto &child : nd.children) {
      int val = -tree_negaalpha_unlimited(*child, -beta, -alpha, func).first;
      if (val > alpha) {
        alpha = val;
        opt = child->bd;
      }
      if (alpha >= beta) return std::make_pair(alpha, opt);
    }
    return std::make_pair(alpha, opt);
  }
}
