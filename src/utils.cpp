#include "utils.hpp"
#include <Color.hpp>
#include <cmath>

raylib::Color lerpColor(const raylib::Color a, const raylib::Color b,
                        const float t) {
  return raylib::Color(std::lerp(a.r, b.r, t), std::lerp(a.g, b.g, t),
                       std::lerp(a.b, b.b, t), std::lerp(a.a, b.a, t));
}

raylib::Color lerpColor3(const raylib::Color a, const raylib::Color b,
                         const raylib::Color c, const float t) {
  if (t < 0.5) {
    return lerpColor(a, b, 2.f * t);
  } else {
    return lerpColor(b, c, 2.f * (t - 0.5f));
  }
}

float sigmoid(float x) { return 1.f / (1.f + std::exp(-x)); }

std::string trim(const std::string &str) {
  auto first = str.find_first_not_of(' ');
  if (first == std::string::npos)
    return "";
  auto last = str.find_last_not_of(' ');
  return str.substr(first, last - first + 1);
}
