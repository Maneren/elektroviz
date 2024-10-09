#include "utils.hpp"
#include <cmath>

raylib::Color lerpColor(const raylib::Color a, const raylib::Color b,
                        const float t) {
  return raylib::Color(std::lerp(a.r, b.r, t), std::lerp(a.g, b.g, t),
                       std::lerp(a.b, b.b, t), std::lerp(a.a, b.a, t));
}
