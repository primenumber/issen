#include "utils.hpp"

#include <iostream>

#include "value.hpp"
#include "bit_manipulations.hpp"
#include "state.hpp"
#include "line.hpp"

namespace utils {

void init_all() {
  value::init();
  bit_manipulations::init();
  state::init();
  line::init();
}

std::string to_s(uint64_t bits) {
  bit_board bb(bits);
  std::string res;
  for (int i = 0; i < 8; ++i) {
    for (int j = 0; j < 8; ++j) {
      res += bb[i*8+j] ? 'x' : '.';
    }
    res += '\n';
  }
  return res;
}

std::string to_s(const board &bd) {
  std::string res;
  res += " |abcdefgh\n";
  res += "-+--------\n";
  for (int i = 0; i < 8; ++i) {
    res +=(char)('1' + i);
    res += '|';
    for (int j = 0; j < 8; ++j) {
      if (bd.black.get(i*8+j)) {
        res += 'x';
      } else if (bd.white.get(i*8+j)) {
        res += 'o';
      } else {
        res += '.';
      }
    }
    res += '\n';
  }
  return res;
}

std::pair<board, bool> input() {
  std::string hs;
  std::cin >> hs;
  char hand = hs[0];
  board bd;
  for (int i = 0; i < 8; ++i) {
    std::string line;
    std::cin >> line;
    for (int j = 0; j < 8; ++j) {
      switch(line[j]) {
       case 'x':
        bd.black.set(i*8+j);
        bd.white.reset(i*8+j);
        break;
       case 'o':
        bd.black.reset(i*8+j);
        bd.white.set(i*8+j);
        break;
       default:
        bd.black.reset(i*8+j);
        bd.white.reset(i*8+j);
        break;
      }
    }
  }
  if (hand == 'x') {
    return std::make_pair(bd, true);
  } else {
    return std::make_pair(board::reverse_board(bd), false);
  }
}

std::pair<board, bool> input_ffo() {
  std::string line;
  std::cin >> line;
  board bd;
  for (int i = 0; i < 64; ++i) {
    switch(line[i]) {
     case 'X':
      bd.black.set(i);
      bd.white.reset(i);
      break;
     case 'O':
      bd.black.reset(i);
      bd.white.set(i);
      break;
     default:
      bd.black.reset(i);
      bd.white.reset(i);
      break;
    }
  }
  std::string hand;
  std::cin >> hand;
  if (hand == "Black") {
    return std::make_pair(bd, true);
  } else {
    return std::make_pair(board::reverse_board(bd), false);
  }
}

} // namespace utils
