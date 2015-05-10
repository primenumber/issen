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
  auto tp = tm.endgame_search();
  for (auto p : std::get<0>(tp))
    std::cout << utils::to_s(std::get<0>(p));
  std::cout << "num: " << std::get<1>(tp) << std::endl;
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
