#include "statistic_value_generator.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>
#include <future>
#include <mutex>

#include "board.hpp"
#include "utils.hpp"
#include "subboard.hpp"
#include "value.hpp"
#include "state.hpp"
#include "bit_manipulations.hpp"

namespace sv_gen {

void generate_lsprob() {
  int n;
  std::cin>>n;
  std::vector<int> res(n);
  std::cout << 49*n << std::endl;
  for (int i = 0; i < n; ++i) {
    board bd = utils::input_bd();
    auto v = subboard::serialize(bd);
    std::sort(std::begin(v), std::end(v));
    for (int index : v) {
      std::cout << i << ' ' << index << ' ' << 1 << '\n';
    }
    int offset = subboard::index_max+1;
    std::cout << i << ' ' << offset << ' ' << state::puttable_black_count(bd) << '\n';
    std::cout << i << ' ' << (offset+1) << ' ' << state::puttable_black_count(board::reverse_board(bd)) << '\n';
    std::cout << i << ' ' << (offset+2) << ' ' << 1 << '\n';
    std::cin>>res[i];
  }
  std::cout << n << std::endl;
  for (int r : res) {
    std::cout << r << '\n';
  }
}

std::mutex mlsp2_in;
std::mutex mlsp2_out;

void lsprob2_worker(const int n, const int stride, const int offset) {
  for (int i = offset; i < n; i += stride) {
    std::string line;
    std::unique_lock<std::mutex> lg_in(mlsp2_in);
    std::getline(std::cin, line);
    lg_in.unlock();
    std::stringstream ss_in;
    ss_in << line;
    std::string str;
    ss_in >> str;
    int score;
    ss_in >> score;
    board bd = bit_manipulations::toBoard(str);
    auto v = subboard::serialize(bd);
    std::sort(std::begin(v), std::end(v));
    std::stringstream ss_out;
    for (int index : v) {
      ss_out << index << ' ';
    }
    int pt_p = state::puttable_black_count(bd);
    int pt_o = state::puttable_black_count(board::reverse_board(bd));
    ss_out << pt_p << ' '
        << pt_o << ' '
        << score << '\n';
    std::string out = ss_out.str();
    std::lock_guard<std::mutex> lg_out(mlsp2_out);
    std::cout << out;
  }
}

void generate_lsprob2() {
  int n;
  std::cin>>n;
  std::cout << n << std::endl;
  int th_cnt = std::thread::hardware_concurrency();
  std::vector<std::future<void>> vf;
  for (int i = 0; i < th_cnt; ++i) {
    vf.push_back(std::async(std::launch::async, lsprob2_worker, n, th_cnt, i));
  }
  for (auto && f : vf) f.get();
}

} // namespace sv_gen
