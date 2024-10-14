#pragma once
#include "Charge.hpp"
#include <Color.hpp>
#include <Vector2.hpp>
#include <vector>

namespace field_line {

// Bezier segment
struct Segment {
  raylib::Vector2 p0;
  raylib::Vector2 p1;
  raylib::Vector2 p2;
  raylib::Vector2 p3;
};

} // namespace field_line

class FieldLine {
public:
  FieldLine(const raylib::Vector2 &start_position,
            const raylib::Vector2 &start_direction, const raylib::Color &color)
      : color(color), start_position(start_position),
        start_direction(start_direction), points{} {};
  virtual ~FieldLine() = default;
  virtual void update(const float timeDelta, const float elapsedTime,
                      const std::vector<Charge> &charges);
  virtual void draw() const;

private:
  raylib::Color color;
  raylib::Vector2 start_position;
  raylib::Vector2 start_direction;
  std::vector<raylib::Vector2> points;
};
