#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "picojson/picojson.h"

#include "board.hpp"
#include "state.hpp"
#include "hand.hpp"
#include "utils.hpp"
#include "tree_manager.hpp"
#include "generate.hpp"
#include "statistic_value_generator.hpp"
#include "play.hpp"
#include "ggs_archive_parser.hpp"
#include "iddfs.hpp"
#include "bit_manipulations.hpp"

void ffotest() {
  board bd;
  bool is_black;
  std::tie(bd, is_black) = utils::input_ffo();
  std::cout << utils::to_s(bd);
  /*
  std::cout << value::statistic_value(bd) << std::endl;
  tree_manager::tree_manager tm(bd, is_black);
  auto tp = tm.endgame_search();
  for (auto p : std::get<0>(tp))
    std::cout << to_s(std::get<1>(p));
  std::cout << std::endl;
  std::cout << utils::to_s(std::get<0>(std::get<0>(tp).back()));
  std::cout << "num: " << std::get<1>(tp) << std::endl;
  */
  GameSolver egs(10000001);
  int pt = egs.iddfs(bd);
  std::cout << pt << std::endl;
}

void obftest() {
  for (int i = 0; i < 1000; ++i) {
    std::cout << "line: " << (i+1) << std::endl;
    board bd;
    int num;
    bool is_black;
    std::tie(bd, num, is_black) = utils::input_obf();
    std::cerr << "num: " << num << ", sscore: " << value::statistic_value(bd) << std::endl;
    GameSolver egs(10000001);
    int pt = egs.iddfs(bd);
    if (pt != num*100) {
      std::cerr << utils::to_s(bd);
      std::cerr << pt << ' ' << num << std::endl;
    }
  }
}

void generate_record() {
  board bd = utils::input_ffo().first;
  int n;
  std::cin >> n;
  generator::generate_record(bd, n);
}

void generate_lsprob(const std::vector<std::string> &args) {
  int n = std::stoi(args[2]);
  sv_gen::generate_lsprob_input(n);
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

int main(int argc, char **argv) {
  utils::init_all();
  std::vector<std::string> args(argc);
  for (int i = 0; i < argc; ++i)
    args[i] = argv[i];
  if (std::count(std::begin(args), std::end(args), "--ffotest"))
    ffotest();
  else if (std::count(std::begin(args), std::end(args), "--obftest"))
    obftest();
  else if (std::count(std::begin(args), std::end(args), "--gen-record"))
    generate_record();
  else if (std::count(std::begin(args), std::end(args), "--gen-lsprob"))
    generate_lsprob(args);
  else if (std::count(std::begin(args), std::end(args), "--ggs-parse"))
    ggs_archive_parser();
  else if (std::count(std::begin(args), std::end(args), "--record-view"))
    record_view();
  else
    play();
  return 0;
}
