#include "subboard.hpp"

#include <algorithm>
#include <vector>

#include "bit_manipulations.hpp"

namespace subboard {

std::vector<std::vector<int>> base3_to_index;
std::vector<int> index_size;
int index_max;

constexpr std::array<int, 11> stones = {
  8, 8, 8, // horizontal
  8, 7, 6, 5, 4, // diagonal
  10, // edge
  9, 10 // corner
};

std::vector<int> index_begin;

constexpr int pow3(int index) {
  return index ? 3 * pow3(index - 1) : 1;
}

std::vector<int> int2digits(int num, int size) {
  std::vector<int> digit(size);
  for (int k = 0; k < size; ++k) {
    digit[k] = num % 3;
    num /= 3;
  }
  return digit;
}

int digits2int(std::vector<int> digits) {
  int res = 0;
  for (int i = digits.size() - 1; i >= 0; --i) {
    res *= 3;
    res += digits[i];
  }
  return res;
}

int rev_num(int num, int size) {
  std::vector<int> digits = int2digits(num, size);
  std::vector<int> rev(size);
  std::reverse_copy(begin(digits), end(digits), begin(rev));
  return digits2int(rev);
}

void init() {
  using std::vector;
  using std::begin;
  using std::end;
  base3_to_index.resize(11);
  index_size.resize(11);
  for (int i = 3; i <= 10; ++i) {
    base3_to_index[i].resize(pow3(i));
    vector<int> nums;
    for (int j = 0; j < pow3(i); ++j) {
      nums.push_back(std::min(j, rev_num(j, i)));
    }
    std::sort(begin(nums), end(nums));
    nums.erase(std::unique(begin(nums), end(nums)), end(nums));
    index_size[i] = nums.size();
    for (int j = 0; j < pow3(i); ++j) {
      int k = std::min(j, rev_num(j, i));
      base3_to_index[i][j] =
          std::lower_bound(begin(nums), end(nums), k) - begin(nums);
    }
  }
  index_begin.resize(stones.size());
  index_begin[0] = 0;
  for (int i = 1; i < (int)stones.size(); ++i) {
    index_begin[i] = index_begin[i-1] + index_size[stones[i-1]];
  }
  index_max = index_begin.back() + index_size[stones.back()] - 1;
}

int to_index(uint64_t black, uint64_t white, int size) {
  return base3_to_index[size][bit_manipulations::toBase3_8(black, white)];
}

int get_index_horizontal(const board &bd, int index) {
  int hi = std::min(index, 7-index);
  return index_begin[hi-1] +
      to_index(bd.black() >> (index*8), bd.white() >> (index*8), 8);
}

int get_index_vertical(const board &bd, int index) {
  return get_index_horizontal(bit_manipulations::flipDiagA1H8(bd), index);
}

int get_index_diagonal_A1H8(const board &bd, int index) {
  board rtbd = bit_manipulations::pseudoRotate45clockwise(bd);
  uint8_t black_bit, white_bit;
  if (index >= 0) {
    black_bit = (rtbd.black() >> (index*8)) & (0xFF >> index);
    white_bit = (rtbd.white() >> (index*8)) & (0xFF >> index);
  } else {
    black_bit = ((rtbd.black() >> ((8+index)*8))  & 0xFF) >> (8+index);
    white_bit = ((rtbd.white() >> ((8+index)*8)) & 0xFF) >> (8+index);
  }
  return index_begin[3 + std::abs(index)] +
      to_index(black_bit, white_bit, 8-std::abs(index));
}

int get_index_diagonal_A8H1(const board &bd, int index) {
  return get_index_diagonal_A1H8(bit_manipulations::flipVertical(bd), index);
}

// *a****b* -> 000000ab
uint8_t get_xbit(uint8_t line) {
  return (uint8_t)((line & 0b01000010) * 17) >> 5;
}

uint16_t get_edge_bits(const bit_board &bbd) {
  uint16_t bit = bbd & 0xFF;
  return bit | (uint16_t)get_xbit(bbd >> 8) << 8;
}

int get_index_edge(const board &bd) {
  return index_begin[8] +
      to_index( get_edge_bits(bd.black()), get_edge_bits(bd.white()), 10);
}

uint16_t get_corner_3x3_bits(const bit_board &bbd) {
  uint16_t bit = 0;
  for (int i = 0; i < 3; ++i)
    bit |= (uint16_t)((bbd >> (i*8)) & 0b111) << (3 * i);
  return bit;
}

int get_index_corner_3x3(const board &bd) {
  return index_begin[9] + to_index(get_corner_3x3_bits(bd.black()),
      get_corner_3x3_bits(bd.white()), 9);
}

uint16_t get_corner_2x5_bits(const bit_board &bbd) {
  uint16_t bit = 0;
  for (int i = 0; i < 2; ++i)
    bit |= (uint16_t)((bbd >> (i*8)) & 0b11111) << (5 * i);
  return bit;
}

int get_index_corner_2x5(const board &bd) {
  return index_begin[10] + to_index(get_corner_2x5_bits(bd.black()),
      get_corner_2x5_bits(bd.white()), 10);
}

std::vector<int> serialize(const board &bd) {
  std::vector<int> cols;
  for (int i = 1; i < 7; ++i) {
    cols.push_back(get_index_horizontal(bd, i));
    cols.push_back(get_index_vertical(bd, i));
  }
  for (int i = -4; i <= 4; ++i) {
    cols.push_back(get_index_diagonal_A1H8(bd, i));
    cols.push_back(get_index_diagonal_A8H1(bd, i));
  }
  for (int i = 0; i < 4; ++i) {
    board tmp = bd;
    if (i & 1) tmp = bit_manipulations::rotate90clockwise(tmp);
    if (i & 2) tmp = bit_manipulations::rotate180(tmp);
    cols.push_back(get_index_edge(tmp));
    cols.push_back(get_index_corner_3x3(tmp));
  }
  for (int i = 0; i < 8; ++i) {
    board tmp = bd;
    if (i & 1) tmp = bit_manipulations::rotate90clockwise(tmp);
    if (i & 2) tmp = bit_manipulations::rotate180(tmp);
    if (i & 4) tmp = bit_manipulations::flipVertical(tmp);
    cols.push_back(get_index_corner_2x5(tmp));
  }
  return cols;
}

} // namespace subboard
