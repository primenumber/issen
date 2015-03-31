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
int dfs_unlimited(const board &bd,
    int alpha, int beta, const Func &func, bool is_pass = false);

template <typename Func>
int dfs_unlimited_impl_sort_search(const board &bd,
    int alpha, int beta, const Func &func, uint64_t bits) {
  std::vector<std::pair<int, board>> v;
  v.reserve(_popcnt64(bits));
  for (const auto &nx : state::next_states(bd, bits)) {
    v.emplace_back(func(nx), nx);
  }
  std::sort(std::begin(v), std::end(v),
      [](const std::pair<int, board> &lhs,
          const std::pair<int, board> &rhs) {
        return lhs.first > rhs.first;
      });
  for (const auto &nx : v){
    alpha = std::max(alpha,
        -dfs_unlimited(nx.second, -beta, -alpha, func));
    if (alpha >= beta) return alpha;
  }
  return alpha;
}


template <typename Func>
int dfs_unlimited(const board &bd,
    int alpha, int beta, const Func &func, bool is_pass) {
  uint64_t bits = state::puttable_black(bd);
  if (bits != 0) {
    if (_popcnt64(bits) > 3) {
      return dfs_unlimited_impl_sort_search(bd, alpha, beta, func, bits);
    } else {
      for (const auto &nx : state::next_states(bd, bits)) {
        alpha = std::max(alpha,
            -dfs_unlimited(nx, -beta, -alpha, func));
        if (alpha >= beta) return alpha;
      }
    }
    return alpha;
  } else {
    if (is_pass) {
      return func(bd);
    } else {
      board rev(bd, reverse_construct_t());
      return std::max(alpha,
          -dfs_unlimited(board::reverse_board(bd),
              -beta, -alpha, func, true));
    }
  }
}

template <typename Func>
int tree_negaalpha_unlimited(node &nd,
    int alpha, int beta, const Func &func) {
  if (nd.children.empty()) {
    return dfs_unlimited(nd.bd, alpha, beta, func);
  } else {
    for (auto &child : nd.children) {
      alpha = std::max(alpha,
          -tree_negaalpha_unlimited(*child, -beta, -alpha, func));
      if (alpha >= beta) return alpha;
    }
    return alpha;
  }
}
