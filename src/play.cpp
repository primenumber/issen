#include <iostream>
#include <string>

#include <boost/optional.hpp>

#include "play.hpp"

void play_hand(hand h) {
  std::cout << "{\"type\":\"play\",\"hand\":\"" << to_s(h) << "\"}" << std::endl;
}

boost::optional<hand> get_opponent_play() {
  using picojson::object;
  std::string line;
  std::getline(std::cin, line);
  std::cerr << line << std::endl;
  picojson::value v;
  picojson::parse(v, line);
  std::string type = v.get<object>()["type"].get<std::string>();
  if (type != "play") return boost::none;
  std::string hand_str = v.get<object>()["hand"].get<std::string>();
  return to_hand(hand_str);
}

void play() {
  using std::string;
  bool is_black = true;
  bool my_color;
  tree_manager::tree_manager tm(board::initial_board(), is_black);
  string color;
  std::getline(std::cin, color);
  my_color = (color == "Black");
  std::cerr << utils::to_s(tm.get_board()) << std::endl;
  while (!state::is_gameover(tm.get_board())) {
    board nx;
    if (my_color == is_black) {
      std::cerr << "me" << std::endl;
      hand h;
      std::tie(nx, h, std::ignore) = tm.normal_search();
      play_hand(h);
      tm.play(nx);
    } else {
      std::cerr << "opponent" << std::endl;
      if (boost::optional<hand> h = get_opponent_play()) {
        nx = board::reverse_board(
            (*h != PASS) ?
              state::put_black_at(tm.get_board(), *h/8, *h%8) :
              tm.get_board());
        tm.play(nx);
      } else {
        break;
      }
    }
    std::cerr << utils::to_s(tm.get_board()) << std::endl;
    is_black = !is_black;
  }
  std::cout << "{\"type\":\"gameset\"}" << std::endl;
}
