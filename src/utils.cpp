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

std::string trim(const std::string &str) {
  auto first = str.find_first_not_of(' ');
  if (first == std::string::npos)
    return "";
  auto last = str.find_last_not_of(' ');
  return str.substr(first, last - first + 1);
}

// adapted from raylib/rshapes.c to allow float threashold
bool CheckCollisionPointLine(Vector2 point, Vector2 p1, Vector2 p2,
                             float threshold) {
  bool collision = false;

  float dxc = point.x - p1.x;
  float dyc = point.y - p1.y;
  float dxl = p2.x - p1.x;
  float dyl = p2.y - p1.y;
  float cross = dxc * dyl - dyc * dxl;

  if (fabsf(cross) < (threshold * fmaxf(fabsf(dxl), fabsf(dyl)))) {
    if (fabsf(dxl) >= fabsf(dyl))
      collision = (dxl > 0) ? ((p1.x <= point.x) && (point.x <= p2.x))
                            : ((p2.x <= point.x) && (point.x <= p1.x));
    else
      collision = (dyl > 0) ? ((p1.y <= point.y) && (point.y <= p2.y))
                            : ((p2.y <= point.y) && (point.y <= p1.y));
  }

  return collision;
}
