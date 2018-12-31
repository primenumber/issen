#pragma once
#include "hand.hpp"

struct Result {
  hand h;
  int value;
  Result() : h(NOMOVE), value(-64) {}
  Result(hand h, int value) : h(h), value(value) {}
};

inline bool operator<(const Result &lhs, const Result &rhs) {
  return lhs.value < rhs.value;
}

