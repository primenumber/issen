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
#include "ggs_archive_parser.hpp"
#include "gamesolver.hpp"
#include "bit_manipulations.hpp"
#include "book.hpp"

void ffotest() {
  board bd;
  bool is_black;
  std::tie(bd, is_black) = utils::input_ffo();
  std::cout << utils::to_s(bd);
  boost::timer::cpu_timer timer;
  GameSolver egs(10000001);
  GameSolverParam param = {true, true, true, true};
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
    GameSolverParam param = {false, false, true, true};
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
    GameSolverParam param = {false, false, true, true};
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
    if (bit_manipulations::stone_sum(bd) == 54) {
      std::cout << value::statistic_value(bd) << std::endl;
    }
    hand h = to_hand(record.substr(i*2, 2));
    if (h != PASS)
      bd = state::move(bd, h/8, h%8);
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

void book81(const std::vector<std::string> &args) {
  int n = std::stoi(args[2]);
  int m = std::stoi(args[3]);
  generator::book_81(n, m);
}

void think_impl(const board &bd) {
  std::cerr << utils::to_s(bd) << std::flush;
  GameSolver gs(1000001);
  hand h;
  int score;
  if (bit_manipulations::stone_sum(bd) < 12) {
    std::tie(h, score) = book::book.think(bd);
  } else if (bit_manipulations::stone_sum(bd) < 42) {
    std::tie(h, score) = gs.think(bd, {true, true, false, true}, 12);
  } else {
    std::tie(h, score) = gs.think(bd, {true, true, true, false}, 64 - bit_manipulations::stone_sum(bd) - 4);
  }
  std::cout << to_s(h) << ' ' << score << std::endl;
}

void think(const std::vector<std::string> &args) {
  board bd = utils::line_to_bd(args[2]);
  if (args[3] == "White") bd = board::reverse_board(bd);
  think_impl(bd);
}

void think_base81(const std::vector<std::string> &args) {
  board bd = bit_manipulations::toBoard(args[2]);
  think_impl(bd);
}

void think_solve(const std::vector<std::string> &args) {
  board bd = bit_manipulations::toBoard(args[2]);
  std::cerr << utils::to_s(bd) << std::flush;
  GameSolver gs(1000001);
  int score = gs.solve(bd, {true, true, true, true});
  std::cout << score << std::endl;
}

void play(const std::vector<std::string> &args) {
  board bd = board::initial_board();
  bool my_color = args[2] == "black";
  bool turn = true;
  GameSolver gs(1000001);
  while (!state::is_gameover(bd)) {
    std::cout << utils::to_s(bd) << std::endl;
    hand h;
    if (my_color == turn) {
      int score;
      if (bit_manipulations::stone_sum(bd) < 12) {
        std::tie(h, score) = book::book.think(bd);
      } else if (bit_manipulations::stone_sum(bd) < 42) {
        std::tie(h, score) = gs.think(bd, {true, true, false, true}, 12);
      } else {
        std::tie(h, score) = gs.think(bd, {true, true, true, false}, 64 - bit_manipulations::stone_sum(bd) - 4);
      }
      std::cout << to_s(h) << ' ' << score << std::endl;
    } else {
      std::string str;
      while (true) {
        std::cin >> str;
        try {
          h = to_hand(str);
          if (h == PASS) {
            if (state::mobility_pos(bd)) {
              throw "invalid pass";
            }
          } else {
            if (((state::mobility_pos(bd) >> h) & 1) == 0) {
              throw "invalid move";
            }
          }
          break;
        } catch(const char *e) {
          std::cerr << e << std::endl;
          continue;
        }
      }
    }
    if (h == PASS) {
      bd = board::reverse_board(bd);
    } else {
      bd = state::move(bd, h);
    }
    turn = !turn;
  }
  std::cout << utils::to_s(bd) << std::endl;
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
  else if (has_opt(args, "--book-base81"))
    book81(args);
  else if (has_opt(args, "--check-base81"))
    check_score();
  else if (has_opt(args, "--ggs-parse"))
    ggs_archive_parser();
  else if (has_opt(args, "--record-view"))
    record_view();
  else if (has_opt(args, "--think-solve"))
    think_solve(args);
  else if (has_opt(args, "--think-base81"))
    think_base81(args);
  else if (has_opt(args, "--think"))
    think(args);
  else if (has_opt(args, "--play"))
    play(args);
  return 0;
}
