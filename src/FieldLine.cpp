#include "FieldLine.hpp"
#include "Vector2.hpp"
#include "defs.hpp"
#include "field.hpp"
#include "parallel.hpp"
#include "raylib.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <optional>
#include <ranges>
#include <span>

namespace views = std::views;
namespace ranges = std::ranges;

/**
 * @brief Calculate a field line given set of parameters
 *
 * @param charges Reference to span of factors in the field.
 * @param start_point Starting point of the line.
 * @param field_function Function the describes the "direction" of the field
 * in given point.
 * @param end_point_function Return the end point if the line should end
 * already.
 * @param direction Direction of the line relative to the field function.
 * @return the line
 */
FieldLines::Line calculate_line(
    const raylib::Vector2 &start_point, const raylib::Vector2 &start_direction,
    const std::function<raylib::Vector2(raylib::Vector2)> &field_function,
    const std::function<std::optional<raylib::Vector2>(raylib::Vector2)>
        &end_point_function,
    const float direction, const float zoom, const raylib::Vector2 target) {
  constexpr size_t STEPS = 10000;
  //  PERF: this is slow as hell, because if unzoomed it has to render more than
  //  hundred thousand points

  std::vector<Vector2> points;
  points.reserve(STEPS);

  points.push_back(world_to_screen(start_point));

  raylib::Vector2 position = start_point + start_direction;

  for (const auto _ : views::iota(0uz, STEPS)) {
    points.push_back(world_to_screen(position));

    auto sample = field_function(position) * direction;
    auto next_position = position + sample.Scale(0.1f / sample.Length() / zoom);

    // Stop if next_position is too far from camera
    if ((next_position - target).LengthSqr() > 80.f / zoom) {
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

void FieldLines::update(const std::span<const Charge> &charges,
                        const float zoom, const raylib::Vector2 target) {
  field_lines.clear();
  equipotencial_lines.clear();

  auto field_function = [charges](auto point) {
    return field::E(point, charges);
  };

  auto end_point_function =
      [charges](raylib::Vector2 point) -> std::optional<raylib::Vector2> {
    auto charge = std::ranges::find_if(charges, [point](auto &charge) {
      return point.CheckCollision(charge.position(), 0.01f);
    });
    return charge != charges.end() ? std::optional(charge->position())
                                   : std::nullopt;
  };

  // count number of positive vs negative charges
  auto positive = ranges::count_if(
      charges, [](auto &charge) { return charge.strength() > 0.f; });

  auto direction = (positive >= 0.f) ? 1.f : -1.f;

  auto world_target = screen_to_world(target, zoom);

  for (const auto &[i, charge] : charges | views::enumerate | views::as_const) {
    // Start with slight offset to align less with axis and other charges
    float initial_angle_offset = 0.1f;

    auto offset = raylib::Vector2{0.f, 0.01f}.Rotate(initial_angle_offset);

    for (size_t j = 0; j < lines_per_charge; ++j) {
      offset = offset.Rotate(2 * std::numbers::pi_v<float> / lines_per_charge);

      auto line =
          calculate_line(charge.position(), offset, field_function,
                         end_point_function, direction, zoom, world_target);

      field_lines.push_back(line);
    }
  }
}

void FieldLines::draw() const {
  for (const auto &line : field_lines) {
    draw_line(line);
  }
}

void FieldLines::draw_line(const Line &line) const {
  color.DrawLineStrip(const_cast<Vector2 *>(line.data()), line.size());
}
