#pragma once

#include <string>
#include <utility>

#include "board.hpp"

namespace utils {

void init_all();
std::string to_s(uint64_t);
std::string to_s(const board &);
std::pair<board, bool> input();
std::pair<board, bool> input_ffo();

} // namespace utils
