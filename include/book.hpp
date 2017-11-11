#pragma once
#include <unordered_map>
#include <vector>
#include <tuple>
#include "board.hpp"
#include "hand.hpp"

namespace book {

void init();

class Book {
 public:
  int lookup(const board bd) const {
    auto itr = table.find(bd);
    if (itr != std::end(table)) {
      return itr->second;
    } else {
      throw "not found";
    }
  }
  void set(const board bd, int score) {
    table[bd] = score;
  }
  std::tuple<hand, int> think(const board) const;
 private:
  std::unordered_map<board, int> table;
};

extern Book book;

std::vector<board> expand_with_think(const board bd, int depth);

} // namespace book
