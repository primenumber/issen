#include "treesearch.hpp"

void expand(node &nd, const board &bd) {
  for (auto nx : state::next_states(bd)) {
    auto child = std::make_unique<node>(nx);
    child->value.push_back(value::value(nx));
    nd.chilidren.push_back(child);
  }
  std::sort(begin(nd.children), end(nd.children),
      [](auto lhs, auto rhs) { return lhs->value.front() < rhs->value.front(); });
}
