#include "utils.hpp"
#include <string_view>

std::string_view trim(const std::string_view str) {
  auto first = str.find_first_not_of(' ');
  if (first == std::string::npos)
    return "";
  auto last = str.find_last_not_of(' ');
  return str.substr(first, last - first + 1);
}
