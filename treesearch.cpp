#include <algorithm>

#include "treesearch.hpp"

namespace treesearch {

void expand(node &nd, const board &bd) {
  for (auto nx : state::next_states(bd)) {
    auto child = std::make_unique<node>(nx);
    child->value.push_back(value::value(nx));
    nd.children.push_back(std::move(child));
  }
  std::sort(begin(nd.children), end(nd.children),
      [](std::unique_ptr<node> &lhs, std::unique_ptr<node> &rhs) {
        return lhs->value.front() < rhs->value.front();
      });
}

} // namespace treesearch
