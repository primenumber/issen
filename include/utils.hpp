#pragma once

#include <string>
#include <utility>
#include <tuple>

#include "board.hpp"

namespace utils {

void init_all();
std::string to_s(uint64_t);
std::string to_s(const board &);
std::string to_s_ffo(const board &);
std::pair<board, bool> input();
std::pair<board, bool> input_ffo();
std::tuple<board, int, bool> input_obf();

} // namespace utils
