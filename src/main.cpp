#include <iostream>
#include <string>
#include <tuple>
#include <vector>

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
  int pt = egs.iddfs(bd, false, true);
  std::cout << pt << std::endl;
  std::cout << timer.format() << std::endl;
}

void obftest() {
  int fail = 0;
  boost::timer::cpu_timer timer;
  for (int i = 0; i < 1000; ++i) {
    std::cout << "line: " << (i+1) << std::endl;
    board bd;
    int num;
    bool is_black;
    std::tie(bd, num, is_black) = utils::input_obf();
    std::cerr << "num: " << num << ", sscore: " << value::statistic_value(bd) << std::endl;
    GameSolver egs(1000001);
    int pt = egs.iddfs(bd);
    if (pt != num) {
      std::cerr << utils::to_s(bd);
      std::cerr << pt << ' ' << num << std::endl;
      ++fail;
    }
  }
  std::cout << "fail: " << fail << std::endl;
  std::cout << timer.format() << std::endl;
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
    generator::solve_81();
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
