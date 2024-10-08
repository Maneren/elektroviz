#include "utils.hpp"
#include <cmath>

raylib::Color lerpColor(const raylib::Color a, const raylib::Color b,
                        const float t) {
  return raylib::Color(std::lerp(a.r, b.r, t), std::lerp(a.g, b.g, t),
                       std::lerp(a.b, b.b, t), std::lerp(a.a, b.a, t));
}

raylib::Color lerpColor3(const raylib::Color a, const raylib::Color b,
                         const raylib::Color c, const float t) {
  if (t < 0.5) {
    return lerpColor(a, b, t * 2);
  } else {
    return lerpColor(b, c, (t - 0.5) * 2);
  }
}
