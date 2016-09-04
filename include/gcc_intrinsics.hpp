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

inline __m256i _mm256_set_m128i(__m128i hi, __m128i lo) {
  return _mm256_inserti128_si256(_mm256_castsi128_si256(hi), lo, 1);
}
inline __m256i _mm256_setr_m128i(__m128i lo, __m128i hi) {
  return _mm256_inserti128_si256(_mm256_castsi128_si256(hi), lo, 1);
}
#endif
