#include "utils.hpp"

raylib::Color lerp(const raylib::Color a, const raylib::Color b,
                   const float t) {
  return raylib::Color(a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t,
                       a.b + (b.b - a.b) * t, a.a + (b.a - a.a) * t);
}
