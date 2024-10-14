#include "FieldLine.hpp"
#include "defs.hpp"
#include "field.hpp"
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
  constexpr size_t STEPS = 2000;

  auto direction = charges[charge_index].strength() >= 0.f ? 1.f : -1.f;

  points.clear();
  points.reserve(STEPS);

  if (direction == -1.f) {
    return;
  }

  auto step = 1e-12f;

  const auto field_function =
      [&charges, direction](const float time,
                            const raylib::Vector2 &point)  {
    return field::E(point, charges) * direction;
  };

  points.push_back(charges[charge_index].position());
  auto position = start_position;

  for (const size_t i : views::iota(0) | views::take(STEPS)) {
    float t = static_cast<float>(i) / STEPS;
    points.push_back(position);

    auto next_position =
        field_line::Euler<raylib::Vector2>(field_function, position, t, step);

    auto diff = (next_position - position);
    auto dist = (diff).LengthSqr();

    constexpr float MAX_DIST = 0.1f * 0.2f;

    if (dist > MAX_DIST) {
      diff = diff.Scale(MAX_DIST / dist);
      next_position = position + diff;
    }

    auto is_colliding = [&](const auto &charge) {
      return CheckCollisionCircleLine(charge.position(), 0.01f, position,
                                      next_position);
    };

    if (auto charge = std::ranges::find_if(charges, is_colliding);
        charge != charges.end()) {
      points.push_back(charge->position());
      return;
    }

    position = next_position;
  }
}

void FieldLine::draw() const {
  for (const auto &[begin, end] : points | views::transform([](const auto &p) {
                                    return p * GLOBAL_SCALE;
                                  }) | views::adjacent<2>)
    begin.DrawLine(end, 0.6f, raylib::Color::DarkGreen());
}
