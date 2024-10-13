#include "field.hpp"
#include "defs.hpp"

raylib::Vector2 E(const raylib::Vector2 point,
                  const std::vector<Charge> &charges) {
  raylib::Vector2 result{0.f};
  for (const auto &charge : charges) {
    result += charge.E(point);
  }
  return result * K_E;
}

float potential(const raylib::Vector2 point,
                const std::vector<Charge> &charges) {
  float result = 0.0f;
  for (const auto &charge : charges) {
    result += charge.potential(point);
  }

  return result * K_E;
}
