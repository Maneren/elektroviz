#include "field.hpp"
#include <Vector2.hpp>
#include <algorithm>
#include <functional>
#include <ranges>
#include <span>

namespace field {

namespace ranges = std::ranges;
namespace views = std::views;

raylib::Vector2 E(const raylib::Vector2 point,
                  const std::span<const Charge> &charges) {
  auto fields = views::transform(charges, [&](auto &c) { return c.E(point); });
  return ranges::fold_left(fields, raylib::Vector2{0.f}, std::plus<>());
}

float potential(const raylib::Vector2 point,
                const std::span<const Charge> &charges) {
  auto potentials =
      views::transform(charges, [&](auto &c) { return c.potential(point); });
  return ranges::fold_left(potentials, 0.f, std::plus<>());
}

} // namespace field
