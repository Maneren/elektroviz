#pragma once
#include "Charge.hpp"
#include "Position.hpp"
#include "defs.hpp"
#include "raylib.h"
#include "utils.hpp"
#include <Color.hpp>
#include <Functions.hpp>
#include <Vector2.hpp>
#include <cmath>
#include <format>
#include <memory>
#include <print>
#include <span>
#include <utility>

class Probe {
public:
  Probe(
      std::unique_ptr<Position> position,
      const raylib::Color &color,
      float radius = 8.f,
      float length = 60.f
  )
      : radius(radius), length(length), _position(std::move(position)),
        _color(color) {}

  Probe(Probe &&) = default;
  Probe &operator=(Probe &&) = default;

  void update(
      const float timeDelta,
      const double elapsedTime,
      const std::span<Charge> &charges
  );

  template <const bool ONLY_ARROW = false> void draw() const {
    auto position = (*_position)();

    auto sample = this->sample();

    auto color = lerpColor(
        raylib::Color::Gray(),
        _color,
        1.f / (1.f + std::exp(-sample.Length() / 1e10f))
    );

    if constexpr (!ONLY_ARROW) {
      position.DrawCircle(radius, color);

      auto msg = std::format("E = {:.1E} N/C", K_E * sample.Length());
      int width = raylib::MeasureText(msg, FONT_SIZE);
      raylib::DrawText(
          msg.c_str(),
          position.x - width / 2.f,
          position.y - 2 * FONT_SIZE,
          FONT_SIZE,
          color
      );
    }

    auto direction = sample.Normalize();

    auto draw_sample = direction.Scale(length);

    auto tip = position + draw_sample;

    // Calculate arrowhead size
    constexpr float head_scale = 1.f / 5.f;
    constexpr float line_scale = 1.f - head_scale;

    auto head_base = position + direction.Scale(line_scale * length);

    // Draw the line
    position.DrawLine(head_base, color);

    // Calculate points for the arrowhead
    float head_width = length * head_scale / 2.f;
    raylib::Vector2 perpendicular(-direction.y, direction.x);
    auto head_left = head_base + perpendicular.Scale(head_width);
    auto head_right = head_base - perpendicular.Scale(head_width);

    // Draw the triangular arrowhead
    DrawTriangle(head_left, tip, head_right, color);
  }

  raylib::Vector2 position() const { return (*_position)(); }
  raylib::Vector2 sample() const { return _sample; }
  float sample_potencial() const { return _sample_potencial; }

  bool contains(const raylib::Vector2 &point) const {
    return (point - (*_position)()).Length() < radius;
  }

  float radius;
  float length = 50.f;

private:
  std::unique_ptr<Position> _position;
  raylib::Color _color;
  raylib::Vector2 _sample;
  float _sample_potencial;
  int _id;

  friend struct std::formatter<Probe>;
  friend class ProbeRenderer;
};

template <> struct std::formatter<Probe> {
  constexpr auto parse(std::format_parse_context const &ctx) const {
    return ctx.begin();
  }
  template <typename FormatContext>
  auto format(const Probe &p, FormatContext &ctx) const {
    return std::format_to(
        ctx.out(), "Probe(position: {}, sample: {})", *p._position, p._sample
    );
  }
};

class ProbeRenderer {
public:
  ProbeRenderer() = default;
  ProbeRenderer(ProbeRenderer &&) = default;
  ProbeRenderer &operator=(ProbeRenderer &&) = default;

  void draw_to_buffer(const Probe &probe);
  void flush();

private:
  struct Line {
    raylib::Vector2 start;
    raylib::Vector2 end;
    raylib::Color color;
  };

  struct Triangle {
    raylib::Vector2 a;
    raylib::Vector2 b;
    raylib::Vector2 c;
    raylib::Color color;
  };

  std::vector<Line> line_buffer;
  std::vector<Triangle> triangle_buffer;
};
