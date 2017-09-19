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
  while(true) {
    lock lk(mtx1);
    if (stack.empty()) break;
    int i = stack.top();
    stack.pop();
    lk.unlock();
    GameSolverParam param = {false, false, true, false};
    result[i] = std::get<1>(gs.think(vb[i], param, 8));
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
    for (int j = 0; j < l; ++j) {
      if (state::puttable_black(bd) == 0) {
        bd = board::reverse_board(bd);
      }
      if (64 - bit_manipulations::stone_sum(bd) == m) {
        std::cout << bit_manipulations::toBase81(bd) << '\n';
        break;
      }
      hand h = to_hand_500k(record.substr(j*2, 2));
      uint64_t puttable = state::puttable_black(bd);
      if (((puttable >> h) & 1) == 0) {
        break;
      }
      bd = state::put_black_at_rev(bd, h);
    }
  }
}

} // namespace generator
