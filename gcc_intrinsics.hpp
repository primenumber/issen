#pragma once
#ifndef __INTEL_COMPILER
#include <cstdint>

constexpr bool _bittest64(const uint64_t * const bits, int index) {
  return (*bits >> index) & UINT64_C(1);
}
bool _bittestandset64(uint64_t * const, int);
bool _bittestandreset64(uint64_t * const, int);
#endif
