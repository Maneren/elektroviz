#include "field.hpp"
#include "defs.hpp"
#include <print>

raylib::Vector2 eFieldAt(const raylib::Vector2 point,
                         const std::vector<Charge> &charges) {
  raylib::Vector2 result = {0.0f, 0.0f};
  for (const auto &charge : charges) {
    const auto distV = charge.position() - point;
    const auto dist = distV.Length();

    result += distV / (dist * dist * dist) * charge.strength();
  }

  return result * K_E;
}
