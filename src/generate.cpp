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
    int pt = gs.iddfs(bd, false);
    std::string base81 = bit_manipulations::toBase81(bd);
    std::lock_guard<std::mutex> lg(mtx2);
    std::cout << base81 << ' ' << pt << '\n';
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
          state::puttable_black(bd)) {
        que.push(bd);
        break;
      }
      hand h = to_hand(record.substr(j*2, 2));
      if (h != PASS)
        bd = state::put_black_at_rev(bd, h/8, h%8);
      else
        bd = board::reverse_board(bd);
    }
  }
  std::cout << que.size() << std::endl;
  std::vector<std::thread> vt;
  for (int i = 0; i < std::thread::hardware_concurrency(); ++i) {
    vt.emplace_back(worker);
  }
  for (auto &&th : vt) {
    th.join();
  }
}

void solver(const std::vector<board> &vb, std::vector<int> &result, std::stack<int> &stack) {
  using lock = std::unique_lock<std::mutex>;
  GameSolver gs(257);
  while(true) {
    lock lk(mtx1);
    if (stack.empty()) break;
    int i = stack.top();
    stack.pop();
    lk.unlock();
    result[i] = gs.iddfs(vb[i], false);
  }
}

void solve_81() {
  int n;
  std::cin >> n;
  std::vector<board> vb(n);
  std::vector<int> result(n);
  std::stack<int> stack;
  for (int i = 0; i < n; ++i) {
    stack.push(i);
    std::string base81;
    std::cin>>base81;
    vb[i] = bit_manipulations::toBoard(base81);
  }
  std::vector<std::thread> vt;
  for (int i = 0; i < std::thread::hardware_concurrency(); ++i) {
    vt.emplace_back(solver, std::cref(vb), std::ref(result), std::ref(stack));
  }
  for (auto &&th : vt) {
    th.join();
  }
  for (int i = 0; i < n; ++i) {
    std::cout << result[i] << '\n';
  }
  std::cout << std::flush;
}

void to_base81(int m) {
  board init = utils::input_ffo().first;
  int n;
  std::cin >> n;
  std::cout << n << std::endl;
  for (int i = 0; i < n; ++i) {
    board bd = init;
    std::string record;
    int tmp;
    std::cin >> record >> tmp;
    int l = record.size()/2;
    for (int j = 0; j < l; ++j) {
      if (64 - bit_manipulations::stone_sum(bd) == m &&
          state::puttable_black(bd)) {
        std::cout << bit_manipulations::toBase81(bd) << '\n';
        break;
      }
      hand h = to_hand(record.substr(j*2, 2));
      if (h != PASS)
        bd = state::put_black_at_rev(bd, h/8, h%8);
      else
        bd = board::reverse_board(bd);
    }
  }
}

} // namespace generator
