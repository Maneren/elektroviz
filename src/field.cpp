#include "field.hpp"
#include <Vector2.hpp>
#include <algorithm>
#include <functional>
#include <ranges>
#include <span>

namespace field {

namespace ranges = std::ranges;
namespace views = std::views;
namespace placeholders = std::placeholders;

raylib::Vector2 E(const raylib::Vector2 point,
                  const std::span<const Charge> &charges) {
  return ranges::fold_left(
      views::transform(charges, std::bind(&Charge::E, placeholders::_1, point)),
      raylib::Vector2{}, std::plus<>());
}

float potential(const raylib::Vector2 point,
                const std::span<const Charge> &charges) {
  return ranges::fold_left(
      views::transform(charges,
                       std::bind(&Charge::potential, placeholders::_1, point)),
      0.f, std::plus<>());
}

} // namespace field
