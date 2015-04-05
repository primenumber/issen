#include <iostream>
#include <tuple>
#include <vector>

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
  while (!state::is_gameover(bd)) {
    auto tp = tm.endgame_search();
    std::cout << utils::to_s(std::get<0>(tp));
    std::cout << "num: " << std::get<2>(tp) << std::endl;
    bd = std::get<0>(tp);
  }
}

int main(int argc, char **argv) {
  utils::init_all();
  std::vector<std::string> args(argc);
  for (int i = 0; i < argc; ++i)
    args[i] = argv[i];
  if (std::count(std::begin(args), std::end(args), "--ffotest"))
    ffotest();
  return 0;
}
