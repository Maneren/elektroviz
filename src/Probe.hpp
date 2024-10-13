#pragma once
#include "Charge.hpp"
#include "Position.hpp"
#include "defs.hpp"
#include <format>
#include <memory>
#include <raylib-cpp.hpp>

class Probe {
public:
  Probe(std::unique_ptr<Position> position, const raylib::Color &color,
        float radius = 8.f)
      : _position(std::move(position)), color(color), radius(radius) {};

  Probe(Probe &&) = default;
  Probe &operator=(Probe &&) = default;

  void update(const float timeDelta, const float elapsedTime,
              const std::vector<Charge> &charges);

  template <const bool ONLY_ARROW = false> void draw() const {
    auto draw_position = (*_position)() * GLOBAL_SCALE;

    auto sample = this->sample();
    if constexpr (!ONLY_ARROW) {
      draw_position.DrawCircle(radius, color);

      auto msg = std::format("E = {} N/C", sample.Length());
      int width = raylib::MeasureText(msg, FONT_SIZE);
      raylib::DrawText(msg.c_str(), draw_position.x - width / 2.f,
                       draw_position.y - 2 * FONT_SIZE, FONT_SIZE, color);
    }

    auto length = -std::log2f(sample.Length());
    auto direction = sample.Normalize();

    auto draw_sample = direction.Scale(length);

    auto tip = draw_position + draw_sample;

    // Calculate arrowhead size
    constexpr float head_scale = 1.f / 5.f;
    constexpr float line_scale = 1.f - head_scale;

    auto head_base = draw_position + direction.Scale(line_scale * length);

    // Draw the line
    draw_position.DrawLine(head_base, 1, color);

    // Calculate points for the arrowhead
    float head_width = length * head_scale / 2.f;
    raylib::Vector2 perpendicular(-direction.y, direction.x);
    auto head_left = head_base + perpendicular.Scale(head_width);
    auto head_right = head_base - perpendicular.Scale(head_width);

    // Draw the triangular arrowhead
    DrawTriangle(head_left, tip, head_right, color);
  };

  raylib::Vector2 sample() const { return _sample; };

  float radius;

private:
  std::unique_ptr<Position> _position;
  raylib::Color color;
  raylib::Vector2 _sample;

  friend struct std::formatter<Probe>;
};

template <> struct std::formatter<Probe> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const Probe &p, FormatContext &ctx) const {
    return std::format_to(ctx.out(), "Probe(position: {}, sample: {})",
                          *p._position, p._sample);
  }
};
