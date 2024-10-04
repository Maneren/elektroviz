#pragma once
#include "Charge.hpp"
#include "Functions.hpp"
#include "defs.hpp"
#include <format>
#include <print>
#include <raylib-cpp.hpp>

class Probe {
public:
  Probe(const raylib::Vector2 &position, const raylib::Color &color)
      : position(position), color(color) {};
  void update(const std::vector<Charge> &charges);

  template <const float CIRCLE_RADIUS = 8.f, const bool TEXT = false>
  void draw() {
    auto draw_position = position * GLOBAL_SCALE;

    draw_position.DrawCircle(CIRCLE_RADIUS, color);

    auto sample = this->sample();
    if constexpr (TEXT) {
      auto msg = std::format("E = {} V/m", sample.Length());
      int width = raylib::MeasureText(msg, FONT_SIZE);
      raylib::DrawText(msg.c_str(), position.x - width / 2.f,
                       position.y - FONT_SIZE, FONT_SIZE, color);
    }

    sample /= 100000000.;

    auto length = sample.Length();
    auto direction = sample.Normalize();

    auto tip = draw_position + sample;

    // Calculate arrowhead size
    constexpr float head_scale = 1.f / 5.f;
    float head_width = length * head_scale / 2.f;

    auto head_base = draw_position + sample.Scale(1 - head_scale);
    draw_position.DrawLine(head_base, 1, color);

    // Calculate points for the arrowhead
    raylib::Vector2 perpendicular(-direction.y, direction.x);
    auto head_left = head_base + perpendicular.Scale(head_width);
    auto head_right = head_base - perpendicular.Scale(head_width);

    // Draw the triangular arrowhead
    DrawTriangle(head_left, tip, head_right, color);
  };

  raylib::Vector2 sample() const { return _sample; };

private:
  raylib::Vector2 position;
  raylib::Color color;
  raylib::Vector2 _sample;
};
