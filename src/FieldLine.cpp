#include "FieldLine.hpp"
#include "defs.hpp"
#include "field.hpp"
#include "raylib.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <ranges>

namespace views = std::views;

namespace field_line {
// Calculate the next value of the function with Euler's method
//
// @param function The function to calculate in form
// $value_{n+1} = value_n + step * function(time, value_n)$
// @param value Current value
// @param time Current time
// @param step Step size
template <typename T>
T Euler(const std::function<T(float, const T &)> &function, const T &value,
        const float &time, const float &step = 0.01f) {
  return value + function(time, value) * step;
}

} // namespace field_line

void FieldLine::update(const std::vector<Charge> &charges) {
  constexpr size_t STEPS = 10000;

  auto direction = charges[charge_index].strength() >= 0.f ? 1.f : -1.f;

  points.clear();
  points.reserve(STEPS);

  if (direction == -1.f) {
    return;
  }

  auto step = 1e-11f;

  const auto field_function =
      [&charges, direction](const float, const raylib::Vector2 &point) {
        return field::E(point, charges) * direction;
      };

  points.push_back(charges[charge_index].position() * GLOBAL_SCALE);
  auto position = start_position;

  for (const size_t i : views::iota(0) | views::take(STEPS)) {
    float t = static_cast<float>(i) / STEPS;
    points.push_back(position * GLOBAL_SCALE);

    auto next_position =
        field_line::Euler<raylib::Vector2>(field_function, position, t, step);

    // clamp position change to 0.05
    {
      auto diff = (next_position - position);
      auto dist = diff.Length();
      if (dist >= 0.1f) {
        next_position = position + diff.Scale(0.1f / dist);
      }
    }

    // stop if next_position is too far from origin
    if ((next_position - start_position).LengthSqr() > 50.f) {
      break;
    }

    auto is_colliding = [&](const auto &charge) {
      return CheckCollisionCircleLine(charge.position(), 0.01f, position,
                                      next_position);
    };

    if (auto charge = std::ranges::find_if(charges, is_colliding);
        charge != charges.end()) {
      points.push_back(charge->position() * GLOBAL_SCALE);
      return;
    }

    position = next_position;
  }
}

void FieldLine::draw() const {
  // use the direct OpenGL line drawing method
  // TODO: remove the cast once https://github.com/RobLoach/raylib-cpp/pull/333
  // is merged
  color.DrawLineStrip((Vector2 *)points.data(), points.size());
}
