#pragma once
#include "Charge.hpp"
#include <Color.hpp>
#include <Vector2.hpp>
#include <functional>
#include <vector>

class FieldLines {
  using Line = std::vector<Vector2>;

public:
  FieldLines() = default;
  FieldLines(const size_t lines_per_charge,
             const raylib::Color &color = raylib::Color::White())
      : lines_per_charge(lines_per_charge), color(color) {}

  void update(const std::span<const Charge> &charges);
  void draw() const;

private:
  void draw_line(const Line &lines) const;

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
  Line calculate_line(
      const raylib::Vector2 &start_point,
      const raylib::Vector2 &start_direction,
      const std::function<raylib::Vector2(raylib::Vector2)> &field_function,
      const std::function<std::optional<raylib::Vector2>(raylib::Vector2)>
          &end_point_function,
      const float direction = 1.f);

  size_t lines_per_charge;
  raylib::Color color;
  std::vector<Line> field_lines{};
  std::vector<Line> equipotencial_lines{};
};
