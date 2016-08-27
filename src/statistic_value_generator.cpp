#include "statistic_value_generator.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

#include "board.hpp"
#include "utils.hpp"
#include "subboard.hpp"
#include "value.hpp"

namespace sv_gen {

void generate_lsprob() {
  int n;
  std::cin>>n;
  std::vector<int> res(n);
  std::cout << 49*n << std::endl;
  for (int i = 0; i < n; ++i) {
    board bd = utils::input_bd();
    auto v = subboard::serialize(bd);
    std::sort(std::begin(v), std::end(v));
    for (int index : v) {
      std::cout << i << ' ' << index << ' ' << 1 << '\n';
    }
    int offset = subboard::index_max+1;
    std::cout << i << ' ' << offset << ' ' << value::puttable_black_count(bd) << '\n';
    std::cout << i << ' ' << (offset+1) << ' ' << value::puttable_black_count(board::reverse_board(bd)) << '\n';
    std::cout << i << ' ' << (offset+2) << ' ' << 1 << '\n';
    std::cin>>res[i];
  }
  std::cout << n << std::endl;
  for (int r : res) {
    std::cout << r << '\n';
  }
}

} // namespace sv_gen
