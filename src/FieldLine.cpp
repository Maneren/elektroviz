#include "FieldLine.hpp"
#include "defs.hpp"
#include "field.hpp"
#include "raylib.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <optional>
#include <ranges>
#include <span>

namespace views = std::views;

void FieldLines::update(const std::span<const Charge> &charges) {
  field_lines.clear();
  equipotencial_lines.clear();

  auto field_function = [charges](auto point) {
    return field::E(point, charges);
  };

  auto end_point_function =
      [charges](raylib::Vector2 point) -> std::optional<raylib::Vector2> {
    auto charge = std::ranges::find_if(charges, [point](auto &charge) {
      return CheckCollisionPointCircle(point, charge.position(), 0.01f);
    });
    return charge != charges.end() ? std::optional(charge->position())
                                   : std::nullopt;
  };

  for (const auto &[i, charge] : charges | views::enumerate | views::as_const) {
    // Start with slight offset to align less with axis and other charges
    float initial_angle_offset = 0.1f;

    auto offset = raylib::Vector2{0.f, 0.1f}.Rotate(initial_angle_offset);

    for (size_t j = 0; j < lines_per_charge; ++j) {
      offset = offset.Rotate(2 * std::numbers::pi_v<float> / lines_per_charge);

      field_lines.push_back(calculate_line(
          charge.position(), offset, field_function, end_point_function, 1.f));
    }
  }
}

FieldLines::Line FieldLines::calculate_line(
    const raylib::Vector2 &start_point, const raylib::Vector2 &start_direction,
    const std::function<raylib::Vector2(raylib::Vector2)> &field_function,
    const std::function<std::optional<raylib::Vector2>(raylib::Vector2)>
        &end_point_function,
    const float direction) {
  constexpr size_t STEPS = 10000;

  std::vector<Vector2> points;
  points.reserve(STEPS);

  points.push_back(world_to_screen(start_point));

  raylib::Vector2 position = start_point + start_direction;

  for (const auto _ : views::iota(0uz, STEPS)) {
    points.push_back(world_to_screen(position));

    auto sample = field_function(position) * direction;
    auto next_position = position + sample.Clamp(0.f, 0.01f);

    // Stop if next_position is too far from origin
    if ((next_position - start_point).LengthSqr() > 50.f) {
      break;
    }

    if (auto end_point = end_point_function(next_position);
        end_point.has_value()) {
      points.push_back(world_to_screen(end_point.value()));
      break;
    }

    position = next_position;
  }

  return points;
}

void FieldLines::draw() const {
  for (const auto &line : field_lines) {
    draw_line(line);
  }
}

void FieldLines::draw_line(const Line &line) const {
  color.DrawLineStrip(const_cast<Vector2 *>(line.data()), line.size());
}
