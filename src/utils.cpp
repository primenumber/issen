#include "utils.hpp"

#include <iostream>

#include "value.hpp"
#include "bit_manipulations.hpp"
#include "state.hpp"
#include "generate.hpp"
#include "subboard.hpp"

namespace utils {

void init_all() {
  bit_manipulations::init();
  state::init();
  subboard::init();
  value::init();
}

std::string to_s(uint64_t bits) {
  bit_board bb(bits);
  std::string res;
  for (int i = 0; i < 8; ++i) {
    for (int j = 0; j < 8; ++j) {
      res += bb.get(i*8+j) ? 'x' : '.';
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
      if (bd.black().get(i*8+j)) {
        res += 'x';
      } else if (bd.white().get(i*8+j)) {
        res += 'o';
      } else {
        res += '.';
      }
    }
    res += '\n';
  }
  return res;
}

std::string to_s_ffo(const board &bd) {
  std::string res;
  for (int i = 0; i < 8; ++i) {
    for (int j = 0; j < 8; ++j) {
      if (bd.black().get(i*8+j)) {
        res += 'X';
      } else if (bd.white().get(i*8+j)) {
        res += 'O';
      } else {
        res += '-';
      }
    }
  }
  res += '\n';
  return res;
}

std::pair<board, bool> input() {
  std::string hs;
  std::cin >> hs;
  char hand = hs[0];
  bit_board black = 0;
  bit_board white = 0;
  for (int i = 0; i < 8; ++i) {
    std::string line;
    std::cin >> line;
    for (int j = 0; j < 8; ++j) {
      switch(line[j]) {
       case 'x':
        black.set(i*8+j);
        white.reset(i*8+j);
        break;
       case 'o':
        black.reset(i*8+j);
        white.set(i*8+j);
        break;
       default:
        black.reset(i*8+j);
        white.reset(i*8+j);
        break;
      }
    }
  }
  board bd(black, white);
  if (hand == 'x') {
    return std::make_pair(bd, true);
  } else {
    return std::make_pair(board::reverse_board(bd), false);
  }
}

board input_bd() {
  std::string line;
  std::cin >> line;
  bit_board black = 0;
  bit_board white = 0;
  for (int i = 0; i < 64; ++i) {
    switch(line[i]) {
     case 'X':
      black.set(i);
      white.reset(i);
      break;
     case 'O':
      black.reset(i);
      white.set(i);
      break;
     default:
      black.reset(i);
      white.reset(i);
      break;
    }
  }
  return board(black, white);
}

std::pair<board, bool> input_ffo() {
  board bd = input_bd();
  std::string hand;
  std::cin >> hand;
  if (hand == "Black") {
    return std::make_pair(bd, true);
  } else {
    return std::make_pair(board::reverse_board(bd), false);
  }
}

std::tuple<board, int, bool> input_obf() {
  board bd = input_bd();
  std::string hand;
  std::cin >> hand;
  std::string tmp;
  std::getline(std::cin, tmp, ':');
  int num;
  std::cin >> num;
  std::getline(std::cin, tmp);
  if (hand[0] == 'X') {
    return std::make_tuple(bd, num, true);
  } else {
    return std::make_tuple(board::reverse_board(bd), -num, false);
  }
}

} // namespace utils
