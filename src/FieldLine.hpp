#pragma once
#include "Charge.hpp"
#include <Color.hpp>
#include <Vector2.hpp>
#include <vector>

class FieldLine {
public:
  FieldLine(const raylib::Vector2 &start_position, const size_t charge_index,
            const raylib::Color &color)
      : color(color), start_position(start_position),
        charge_index(charge_index) {}
  virtual ~FieldLine() = default;
  virtual void update(const std::vector<Charge> &charges);
  void draw() const;

private:
  raylib::Color color;
  raylib::Vector2 start_position;
  size_t charge_index;
  std::vector<Vector2> points{};
};
