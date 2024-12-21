#include "field.hpp"
#include "defs.hpp"
#include <Vector2.hpp>
#include <algorithm>
#include <functional>
#include <ranges>
#include <span>

namespace field {

namespace ranges = std::ranges;
namespace views = std::views;
namespace placeholders = std::placeholders;

constexpr float FIELD_SCALE = GLOBAL_SCALE * GLOBAL_SCALE;

raylib::Vector2
E(const raylib::Vector2 point, const std::span<const Charge> &charges) {
  return ranges::fold_left(
             views::transform(
                 charges, [&point](auto &charge) { return charge.E(point); }
             ),
             raylib::Vector2{},
             std::plus<>()
         ) *
         FIELD_SCALE;
}

float potential(
    const raylib::Vector2 point, const std::span<const Charge> &charges
) {
  return ranges::fold_left(
             views::transform(
                 charges,
                 [&point](auto &charge) { return charge.potential(point); }
             ),
             0.f,
             std::plus<>()
         ) *
         FIELD_SCALE;
}

} // namespace field
