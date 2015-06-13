#include <cctype>
#include <iostream>
#include <string>
#include <tuple>
#include <boost/optional.hpp>

#include "board.hpp"
#include "hand.hpp"
#include "state.hpp"
#include "value.hpp"
#include "record.hpp"
#include "utils.hpp"

std::tuple<int, int> number(const std::string &s, int i) {
  int n = 0;
  while (i < s.size() && std::isdigit(s[i])) {
    n *= 10;
    n += s[i] - '0';
    ++i;
  }
  return std::make_tuple(n, i);
}

std::tuple<std::string, int> name(const std::string &s, int i) {
  std::string res;
  while (s[i] != '[') {
    res += s[i];
    ++i;
  }
  return std::make_tuple(res, i);
}

std::tuple<std::string, int> data(const std::string &s, int i) {
  std::string res;
  ++i;
  while (s[i] != ']') {
    res += s[i];
    ++i;
  }
  return std::make_tuple(res, ++i);
}

std::tuple<std::string, std::string, int> elem(const std::string &s, int i) {
  std::string type;
  std::tie(type, i) = name(s, i);
  std::string body;
  std::tie(body, i) = data(s, i);
  return std::make_tuple(type, body, i);
}

std::string fix_hand_str(const std::string &ggs_str) {
  std::string res;
  for (auto c : ggs_str) {
    c = std::tolower(c);
    if (std::isalpha(c) || ('1' <= c && c <= '8')) res += c;
  }
  assert(res.size() >= 2);
  if (res[1] == 'a') res[1] = 's';
  return res.substr(0, 2);
}

std::tuple<boost::optional<record::game_record>, int> game(const std::string &s, int i) {
  i += 2;
  board bd = board::initial_board();
  record::game_record gr;
  bool is_black = true;
  bool is_valid = true;
  while(isupper(s[i])) {
    std::string type, body;
    std::tie(type, body, i) = elem(s, i);
    if (type == "BO") {
      int sz = std::get<0>(number(body, 0));
      if (sz != 8) is_valid = false;
    } else if (is_valid && (type == "B" || type == "W")) {
      hand h = to_hand(fix_hand_str(body));
      gr.hands.push_back(h);
      if (h != PASS)
        bd = state::put_black_at_rev(bd, h / 8, h % 8);
      else
        bd = board::reverse_board(bd);
      is_black = !is_black;
    }
  }
  while(s[i] != ';') ++i;
  i += 2;
  if (is_valid && state::is_gameover(bd)) {
    int num = value::fixed_diff_num(bd);
    if (!is_black) num *= -1;
    gr.result = num;
    return std::make_tuple(boost::optional<record::game_record>(gr), i);
  } else {
    return std::make_tuple(boost::none, i);
  }
}

std::vector<record::game_record> parse_line(const std::string &line) {
  int i,n;
  std::tie(n, i) = number(line, 0);
  ++i;
  std::vector<record::game_record> grv;
  for (int cnt = 0; cnt < n; ++cnt) {
    boost::optional<record::game_record> gr_opt;
    std::tie(gr_opt, i) = game(line, i);
    if (gr_opt) {
      grv.push_back(std::move(*gr_opt));
    }
  }
  return grv;
}

void ggs_archive_parser() {
  using std::begin;
  using std::end;
  using std::cout;
  using std::endl;
  cout << utils::to_s_ffo(board::initial_board());
  cout << "Black" << endl;
  std::string line;
  std::vector<record::game_record> grv;
  while(std::getline(std::cin, line)) {
    auto v = parse_line(line);
    grv.insert(end(grv), begin(v), end(v));
  }
  cout << grv.size() << endl;
  for (auto &gr : grv) {
    for (hand h : gr.hands) {
      cout << to_s(h);
    }
    cout << endl;
    cout << gr.result << endl;
  }
}
