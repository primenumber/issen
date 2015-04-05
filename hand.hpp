#pragma once

#include "board.hpp"

using hand = int;

constexpr hand PASS = -1;

hand hand_from_diff(const board &old_b, const board &new_b);
