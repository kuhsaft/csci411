//
// Created by Peter on 1/28/2018.
//

#include <sstream>
#include <algorithm>
#include "string_util.h"

std::list<std::string> splitString(const std::string &input) {
  std::list<std::string> result;
  std::istringstream input_ss(input);
  for (std::string s; input_ss >> s;)
    result.push_back(s);

  return result;
}

std::string joinString(const std::list<std::string> &list, const std::string &delim) {
  std::string output;

  if (list.empty())
    return output;
  else
    output = list.front();

  for (auto it = ++list.begin(), end = list.end(); it != end; ++it) {
    output += delim;
    output += *it;
  }

  return output;
}

void trim(std::string &str) {
  // Trim whitespace left
  str.erase(
      str.begin(),
      std::find_if(str.begin(), str.end(), [](int ch) {
        return !std::isspace(ch);
      })
  );

  // Trim whitespace right
  str.erase(
      std::find_if(str.rbegin(), str.rend(), [](int ch) {
        return !std::isspace(ch);
      }).base(),
      str.end()
  );
}
