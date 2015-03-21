#pragma once
#include "board.hpp"
#include "state.hpp"

namespace value {

constexpr int VALUE_MAX = 100000;

void init();
int value(const board &);

} // namespace value
