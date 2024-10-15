#include "field.hpp"
#include "defs.hpp"
#include <algorithm>
#include <functional>
#include <ranges>

namespace field {

namespace ranges = std::ranges;
namespace views = std::views;

raylib::Vector2 E(const raylib::Vector2 point,
                  const std::vector<Charge> &charges) {
  auto fields = views::transform(charges, [&](auto &c) { return c.E(point); });
  return ranges::fold_left(fields, raylib::Vector2{0.f}, std::plus<>()) * K_E;
}

float potential(const raylib::Vector2 point,
                const std::vector<Charge> &charges) {
  auto potentials =
      views::transform(charges, [&](auto &c) { return c.potential(point); });
  return ranges::fold_left(potentials, 0.f, std::plus<>()) * K_E;
}

} // namespace field
