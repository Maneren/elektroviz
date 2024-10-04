#pragma once
#include "Charge.hpp"
#include "Functions.hpp"
#include "defs.hpp"
#include <format>
#include <ostream>
#include <print>
#include <raylib-cpp.hpp>

class Probe {
public:
  Probe(const raylib::Vector2 &position, const raylib::Color &color)
      : position(position), color(color) {};
  void update(const std::vector<Charge> &charges);

  template <const float size = 8.f, const bool text = false> void draw() {
    auto draw_position = position * GLOBAL_SCALE;
    // draw_position.DrawCircle(size, color);
    auto sample = this->sample();
    if constexpr (text) {
      auto msg = std::format("E = {} V/m", sample.Length());
      int width = raylib::MeasureText(msg, 1);
      raylib::DrawText(msg.c_str(), position.x - width / 2.f,
                       position.y - 4.f * GLOBAL_SCALE, GLOBAL_SCALE * 2,
                       color);
    }

    sample /= 100000000.;

    auto length = sample.Length();
    auto direction = sample.Normalize();

    std::println("Length: {}", length);
    std::println("Direction: {}, {}", direction.x, direction.y);

    auto tip = position + sample;

    // Calculate arrowhead size
    float head_size = length / 5.f;

    auto head_base = tip - (direction * head_size);
    draw_position.DrawLine(head_base, 1, color);

    // Calculate points for the arrowhead
    raylib::Vector2 perpendicular = {-direction.y, direction.x};
    auto head_left = head_base + (perpendicular * head_size / 2.f);
    auto head_right = head_base - (perpendicular * head_size / 2.f);

    // Draw the triangular arrowhead
    DrawTriangle(head_left, head_base, head_right, color);

    std::println("Position: {}, {}, tip: {}, {}, head: {}, {}", position.x,
                 position.y, tip.x, tip.y, head_base.x, head_base.y);
  };

  raylib::Vector2 sample() const { return _sample; };

private:
  raylib::Vector2 position;
  raylib::Color color;
  raylib::Vector2 _sample;
};
