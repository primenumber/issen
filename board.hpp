#pragma once
#include <array>
#include <immintrin.h>

struct bit_board {
  class bit_accessor {
   public:
    constexpr bit_accessor(const uint64_t * d64p, const size_t index) : d64p(d64p), index(index) {}
    operator bool() { return _bittest64((__int64 *)d64p, index); }
    bool operator=(const bool bit) {
      if (bit) _bittestandset64((__int64 *)d64p, index);
      else _bittestandreset64((__int64 *)d64p, index);
      return bit;
    }
   private:
    const uint64_t * d64p;
    const size_t index;
  };
  union {
    uint64_t data;
    std::array<uint8_t, 8> lines;
  };
  bit_board() = default;
  constexpr bit_board(const bit_board &) = default;
  constexpr bit_board(const uint64_t data) : data(data) {}
  constexpr bit_board(const std::array<uint8_t, 8> lines) : lines(lines) {}
  operator uint64_t() { return data; }
  operator uint64_t() const { return data; }
  bit_accessor operator[](const size_t index) {
    return bit_accessor(&data, index);
  }
  bool operator[](const size_t index) const { return get(index); }
  bool get(const size_t index) const { return _bittest64((__int64 *)&data, index); }
  bool set(const size_t index) { return _bittestandset64((__int64 *)&data, index); }
  bool reset(const size_t index) { return _bittestandreset64((__int64 *)&data, index); }
  bool setval(const size_t index, const bool bit) {
    return bit ? set(index) : reset(index);
  }
};

struct reverse_construct_t {};

union board {
  __m128i data;
  struct {
    bit_board black, white;
  };
  std::array<bit_board, 2> colors;
  board() = default;
  board(const board &) = default;
  board(const board & bd, const reverse_construct_t) :
      black(bd.white), white(bd.black) {}
  board(const uint64_t black, const uint64_t white) : black(black), white(white) {}
  board(__m128i data) : data(data) {}
  operator __m128i() { return data; }
  operator __m128i() const { return data; }
  board &operator=(const board &) = default;
  board &operator=(board &&) = default;
  static board initial_board() {
    return board(UINT64_C(0x0000000810000000), UINT64_C(0x0000001008000000));
  }
  static board reverse_board(const board &bd) {
    return board(bd, reverse_construct_t());
  }
};

inline bool operator==(const board &lhs, const board &rhs) {
  return lhs.black == rhs.black &&
      lhs.white == rhs.white;
}

inline bool operator<(const board &lhs, const board &rhs) {
  return (lhs.black == rhs.black) ?
      (lhs.white < rhs.white) : (lhs.black < rhs.black);
}
