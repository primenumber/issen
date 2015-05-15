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

void ffotest() {
  board bd;
  bool is_black;
  std::tie(bd, is_black) = utils::input_ffo();
  std::cout << utils::to_s(bd);
  tree_manager::tree_manager tm(bd, is_black);
  auto tp = tm.endgame_search();
  for (auto p : std::get<0>(tp))
    std::cout << utils::to_s(std::get<0>(p));
  std::cout << "num: " << std::get<1>(tp) << std::endl;
}

void play() {
  using picojson::object;
  bool is_black = true;
  bool my_color;
  tree_manager::tree_manager tm(board::initial_board(), is_black);
  std::string color;
  std::getline(std::cin, color);
  my_color = (color == "Black");
  std::cerr << utils::to_s(tm.get_board()) << std::endl;
  while (!state::is_gameover(tm.get_board())) {
    board nx;
    if (my_color == is_black) {
      std::cerr << "me" << std::endl;
      hand h;
      std::tie(nx, h, std::ignore) = tm.normal_search();
      std::cout << "{\"type\":\"play\",\"hand\":\"" << to_s(h) << "\"}" << std::endl;
      tm.play(nx);
    } else {
      std::cerr << "opponent" << std::endl;
      std::string line;
      std::getline(std::cin, line);
      std::cerr << line << std::endl;
      picojson::value v;
      picojson::parse(v, line);
      std::string hand_str = v.get<object>()["hand"].get<std::string>();
      hand h = to_hand(hand_str);
      nx = board::reverse_board(
          (h != PASS) ?
            state::put_black_at(tm.get_board(), h/8, h%8) :
            tm.get_board());
      tm.play(nx);
    }
    std::cerr << utils::to_s(tm.get_board()) << std::endl;
    is_black = !is_black;
  }
}

int main(int argc, char **argv) {
  utils::init_all();
  std::vector<std::string> args(argc);
  for (int i = 0; i < argc; ++i)
    args[i] = argv[i];
  if (std::count(std::begin(args), std::end(args), "--ffotest"))
    ffotest();
  else
    play();
  return 0;
}
