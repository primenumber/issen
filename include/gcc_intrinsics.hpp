#pragma once
#ifndef __INTEL_COMPILER
#include <cstdint>

constexpr bool _bittest64(const uint64_t * const bits, int index) {
  return (*bits >> index) & UINT64_C(1);
}

constexpr bool _bittestandset64(uint64_t * const bits, int index) {
  bool res = _bittest64(bits, index);
  *bits |= (UINT64_C(1) << index);
  return res;
}

constexpr bool _bittestandreset64(uint64_t * const bits, int index) {
  bool res = _bittest64(bits, index);
  *bits &= ~(UINT64_C(1) << index);
  return res;
}

inline __m128i _mm_setr_epi64x(uint64_t lo, uint64_t hi) {
  return _mm_set_epi64x(hi, lo);
}
#endif
