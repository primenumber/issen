#include <algorithm>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>
#include <random>

#include <boost/timer/timer.hpp>

#include "picojson/picojson.h"

#include "board.hpp"
#include "state.hpp"
#include "hand.hpp"
#include "utils.hpp"
#include "generate.hpp"
#include "statistic_value_generator.hpp"
#include "ggs_archive_parser.hpp"
#include "gamesolver.hpp"
#include "bit_manipulations.hpp"

void ffotest() {
  board bd;
  bool is_black;
  std::tie(bd, is_black) = utils::input_ffo();
  std::cout << utils::to_s(bd);
  boost::timer::cpu_timer timer;
  GameSolver egs(10000001);
  GameSolverParam param = {true, true, true, 25};
  int pt = egs.solve(bd, param);
  std::cout << pt << std::endl;
  std::cout << timer.format() << std::endl;
}

void obftest() {
  int fail = 0;
  boost::timer::cpu_timer timer;
  GameSolver egs(1000001);
  for (int i = 0; i < 1000; ++i) {
    std::cout << "line: " << (i+1) << std::endl;
    board bd;
    int num;
    bool is_black;
    std::tie(bd, num, is_black) = utils::input_obf();
    std::cerr << "num: " << num << ", sscore: " << value::statistic_value(bd) << std::endl;
    GameSolverParam param = {false, false, true, 50};
    int pt = egs.solve(bd, param);
    if (pt != num) {
      std::cerr << utils::to_s(bd);
      std::cerr << pt << ' ' << num << std::endl;
      ++fail;
    }
  }
  std::cout << "fail: " << fail << std::endl;
  std::cout << timer.format() << std::endl;
}

void check_score() {
  int n;
  std::cin >> n;
  GameSolver egs(257);
  std::vector<std::pair<std::string, int>> vp;
  for (int i = 0; i < n; ++i) {
    std::string b81;
    int score;
    std::cin >> b81 >> score;
    vp.emplace_back(b81, score);
  }
  std::random_device rd;
  std::mt19937 mt(rd());
  std::shuffle(std::begin(vp), std::end(vp), mt);
  for (int i = 0; i < n; ++i) {
    std::string b81;
    int score;
    std::tie(b81, score) = vp[i];
    GameSolverParam param = {false, false, true, 50};
    int pt = egs.solve(bit_manipulations::toBoard(b81), param);
    if (pt != score) {
      std::cerr << i << ' ' << b81 << ' ' << pt << ' ' << score << std::endl;
    }
    if ((i % 1000) == 0) std::cerr << '.';
  }
  std::cerr << "end" << std::endl;
}

void record_view() {
  board bd = board::initial_board();
  std::string record;
  std::cin >> record;
  int n = record.size()/2;
  for (int i = 0; i < n; ++i) {
    if (_popcnt64(bd.black()|bd.white()) == 54) {
      std::cout << value::statistic_value(bd) << std::endl;
    }
    hand h = to_hand(record.substr(i*2, 2));
    if (h != PASS)
      bd = state::put_black_at_rev(bd, h/8, h%8);
    else
      bd = board::reverse_board(bd);
  }
  if ((n % 2) == 0) {
    std::cout << utils::to_s(bd);
  } else {
    std::cout << utils::to_s(board::reverse_board(bd));
  }
}

void generate_score(const std::vector<std::string> &args) {
  int n = std::stoi(args[2]);
  generator::generate_score(n);
}

void to_base81(const std::vector<std::string> &args) {
  int n = std::stoi(args[2]);
  generator::to_base81(n);
}

void solve81(const std::vector<std::string> &args) {
  int n = std::stoi(args[2]);
  generator::solve_81(n);
}

template<typename Container>
bool has_opt(Container c, std::string s) {
  return std::count(std::begin(c), std::end(c), s);
}

int main(int argc, char **argv) {
  utils::init_all();
  std::vector<std::string> args(argc);
  for (int i = 0; i < argc; ++i)
    args[i] = argv[i];
  if (has_opt(args, "--ffotest"))
    ffotest();
  else if (has_opt(args, "--obftest"))
    obftest();
  else if (has_opt(args, "--gen-score"))
    generate_score(args);
  else if (has_opt(args, "--to-base81"))
    to_base81(args);
  else if (has_opt(args, "--solve-base81"))
    solve81(args);
  else if (has_opt(args, "--check-base81"))
    check_score();
  else if (has_opt(args, "--gen-lsprob2"))
    sv_gen::generate_lsprob2();
  else if (has_opt(args, "--gen-lsprob"))
    sv_gen::generate_lsprob();
  else if (has_opt(args, "--ggs-parse"))
    ggs_archive_parser();
  else if (has_opt(args, "--record-view"))
    record_view();
  return 0;
}
