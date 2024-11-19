#pragma once
#include "Charge.hpp"
#include <Color.hpp>
#include <Vector2.hpp>
#include <vector>

class FieldLines {
public:
  using Line = std::vector<Vector2>;

  FieldLines() = default;
  FieldLines(
      const size_t lines_per_charge,
      const raylib::Color &color = raylib::Color::White()
  )
      : lines_per_charge(lines_per_charge), color(color) {}

  void update(
      const std::span<const Charge> &charges, const raylib::Vector2 world_target
  );
  void draw() const;

private:
  void draw_line(const Line &lines) const;

  size_t lines_per_charge;
  raylib::Color color;
  std::vector<Line> field_lines{};
  std::vector<Line> equipotencial_lines{};
  float zoom;
};
