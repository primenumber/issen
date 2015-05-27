#include "gcc_intrinsics.hpp"

bool _bittestandset64(uint64_t * const bits, int index) {
  bool res = _bittest64(bits, index);
  *bits |= (UINT64_C(1) << index);
  return res;
}

bool _bittestandreset64(uint64_t * const bits, int index) {
  bool res = _bittest64(bits, index);
  *bits &= ~(UINT64_C(1) << index);
  return res;
}
