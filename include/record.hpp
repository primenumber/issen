#pragma once
#include <vector>

#include "hand.hpp"

namespace record {

struct game_record {
  std::vector<hand> hands;
  int result;
  game_record(const std::vector<hand> &h, int res)
      : hands(h), result(res) {}
  game_record(std::vector<hand> &&h, int res)
      : hands(std::move(h)), result(res) {}
  game_record(const game_record &gr) = default;
  game_record(game_record &&gr) = default;
  game_record() : hands(), result() {};
  game_record &operator=(const game_record &gr) = default;
  game_record &operator=(game_record &&gr) = default;
};

} // namespace record
