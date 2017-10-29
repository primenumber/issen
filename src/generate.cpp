#include "generate.hpp"

#include <cstdio>

#include <random>
#include <mutex>
#include <queue>
#include <iostream>
#include <thread>
#include <stack>
#include <vector>

#include "bit_manipulations.hpp"
#include "state.hpp"
#include "hand.hpp"
#include "utils.hpp"
#include "value.hpp"
#include "gamesolver.hpp"
#include "book.hpp"

namespace generator {

std::queue<board> que;
std::mutex mtx1, mtx2;

void worker() {
  GameSolver gs(65537);
  while (true) {
    std::unique_lock<std::mutex> ul(mtx1);
    if (que.empty()) return;
    board bd = que.front(); que.pop();
    ul.unlock();
    GameSolverParam param = {false, false, false, true};
    int pt = gs.solve(bd, param);
    std::string base81 = bit_manipulations::toBase81(bd);
    std::lock_guard<std::mutex> lg(mtx2);
    std::cout << base81 << ' ' << pt << '\n';
  }
}

std::vector<board> expand_desc(const board &bd, const int depth) {
  if (depth <= 0) {
    return std::vector<board>(1, bd);
  } else if (depth == 1) {
    return state::next_states(bd);
  } else {
    auto nexts = state::next_states(bd);
    std::vector<board> res;
    for (const auto &next : nexts) {
      auto desc = expand_desc(next, depth-1);
      res.insert(std::end(res), std::begin(desc), std::end(desc));
    }
    return res;
  }
}

void generate_score(int m) {
  board init = utils::input_ffo().first;
  int n;
  std::cin >> n;
  for (int i = 0; i < n; ++i) {
    board bd = init;
    std::string record;
    int tmp;
    std::cin >> record >> tmp;
    int l = record.size()/2;
    for (int j = 0; j < l; ++j) {
      if (64 - bit_manipulations::stone_sum(bd) == m &&
          state::mobility_pos(bd)) {
        que.push(bd);
        break;
      }
      hand h = to_hand(record.substr(j*2, 2));
      if (h != PASS)
        bd = state::move(bd, h/8, h%8);
      else
        bd = board::reverse_board(bd);
    }
  }
  std::cout << que.size() << std::endl;
  std::vector<std::thread> vt;
  for (size_t i = 0; i < std::thread::hardware_concurrency(); ++i) {
    vt.emplace_back(worker);
  }
  for (auto &&th : vt) {
    th.join();
  }
}

void solver(const std::vector<board> &vb, std::vector<int> &result, std::stack<int> &stack) {
  using lock = std::unique_lock<std::mutex>;
  GameSolver gs(1025);
  int cnt = 0;
  while(true) {
    lock lk(mtx1);
    if (stack.empty()) break;
    int i = stack.top();
    stack.pop();
    lk.unlock();
    bool flipped = false;
    board bd = vb[i];
    GameSolverParam param = {false, false, true, false};
    int score = value::fixed_diff_num(bd);
    while (!state::is_gameover(bd)) {
      hand h;
      std::tie(h, score) = gs.think(bd, param, 6);
      if (h != PASS)
        bd = state::move(bd, h);
      else
        bd = board::reverse_board(bd);
      flipped = !flipped;
    }
    result[i] = flipped ? -score : score;
    ++cnt;
    if (cnt % 1000 == 0) std::cerr << cnt << std::endl;
  }
}

void solve_81(int depth) {
  std::ios::sync_with_stdio(false);
  int n;
  std::cin >> n;
  std::vector<board> vb;
  std::stack<int> stack;
  int cnt = 0;
  for (int i = 0; i < n; ++i) {
    std::string base81;
    std::cin>>base81;
    board bd = bit_manipulations::toBoard(base81);
    auto desc = expand_desc(bd, depth);
    vb.insert(std::end(vb), std::begin(desc), std::end(desc));
    for (size_t j = 0; j < desc.size(); ++j) {
      stack.push(cnt++);
    }
  }
  std::vector<int> result(vb.size());
  std::vector<std::thread> vt;
  for (size_t i = 0; i < std::thread::hardware_concurrency(); ++i) {
    vt.emplace_back(solver, std::cref(vb), std::ref(result), std::ref(stack));
  }
  for (auto &&th : vt) {
    th.join();
  }
  for (size_t i = 0; i < vb.size(); ++i) {
    std::cout << bit_manipulations::toBase81(vb[i]) << ' ' << result[i] << '\n';
  }
  std::cout << std::flush;
}

void thinker(const std::vector<board> &vb, std::vector<int> &result, std::stack<int> &stack, const int depth) {
  using lock = std::unique_lock<std::mutex>;
  GameSolver gs(65537);
  int cnt = 0;
  while(true) {
    lock lk(mtx1);
    if (stack.empty()) break;
    int i = stack.top();
    stack.pop();
    lk.unlock();
    board bd = vb[i];
    GameSolverParam param = {false, false, true, false};
    int score;
    hand h;
    std::tie(h, score) = gs.think(bd, param, depth);
    result[i] = score;
    ++cnt;
    if (cnt % 100 == 0) std::cerr << cnt << std::endl;
  }
}

void book_81(int expand_depth, int search_depth) {
  std::ios::sync_with_stdio(false);
  int n;
  std::cin >> n;
  std::vector<board> vb;
  std::stack<int> stack;
  int cnt = 0;
  for (int i = 0; i < n; ++i) {
    std::string base81;
    std::cin>>base81;
    board bd = bit_manipulations::toBoard(base81);
    auto desc = book::expand_with_think(bd, expand_depth);
    vb.insert(std::end(vb), std::begin(desc), std::end(desc));
    for (size_t j = 0; j < desc.size(); ++j) {
      stack.push(cnt++);
    }
  }
  std::vector<int> result(vb.size());
  std::vector<std::thread> vt;
  for (size_t i = 0; i < std::thread::hardware_concurrency(); ++i) {
    vt.emplace_back(thinker, std::cref(vb), std::ref(result), std::ref(stack), search_depth);
  }
  for (auto &&th : vt) {
    th.join();
  }
  for (size_t i = 0; i < vb.size(); ++i) {
    std::cout << bit_manipulations::toBase81(vb[i]) << ' ' << result[i] << '\n';
  }
  std::cout << std::flush;
}

hand to_hand_500k(const std::string str) {
  int i = str[1] - '1';
  int j = str[0] - 'A';
  return i * 8 + j;
}

void to_base81(int m) {
  std::ios::sync_with_stdio(false);
  board init = board::initial_board();//= utils::input_ffo().first;
  int n;
  std::cin >> n;
  GameSolver gs(10001);
  for (int i = 0; i < n; ++i) {
    board bd = init;
    std::string record;
    int score;
    std::cin >> record >> score;
    int l = record.size()/2;
    bool black = true;
    for (int j = 0; j < l; ++j) {
      if (state::mobility_pos(bd) == 0) {
        bd = board::reverse_board(bd);
        black = !black;
      }
      if (64 - bit_manipulations::stone_sum(bd) == m) {
        std::cout << bit_manipulations::toBase81(bd) << ' ' << (black ? score : -score) << '\n';
        break;
      }
      hand h = to_hand_500k(record.substr(j*2, 2));
      uint64_t puttable = state::mobility_pos(bd);
      if (((puttable >> h) & 1) == 0) {
        break;
      }
      bd = state::move(bd, h);
      black = !black;
    }
  }
}

} // namespace generator
