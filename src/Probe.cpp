#include "Probe.hpp"
#include "Charge.hpp"
#include "Vector2.hpp"
#include "field.hpp"
#include <span>

void Probe::update(
    const float timeDelta,
    const double elapsedTime,
    const std::span<Charge> &charges
) {
  _position->update(timeDelta, elapsedTime);
  _sample = field::E((*_position)(), charges);
  _sample_potencial = field::potential((*_position)(), charges);
}

void ProbeRenderer::draw_to_buffer(const Probe &probe) {
  auto position = probe.position();

  auto sample = probe.sample();

  auto color = lerpColor(
      raylib::Color::Gray(),
      probe._color,
      1.f / (1.f + std::exp(-sample.Length() / 1e10f))
  );

  auto direction = sample.Normalize();

  auto draw_sample = direction.Scale(probe.length);

  auto tip = position + draw_sample;

  // Calculate arrowhead size
  constexpr float head_scale = 1.f / 5.f;
  constexpr float line_scale = 1.f - head_scale;

  auto head_base = position + draw_sample.Scale(line_scale);

  // Draw the line
  line_buffer.emplace_back(position, head_base, color);

  // Calculate points for the arrowhead
  float head_width = probe.length * head_scale / 2.f;
  raylib::Vector2 perpendicular(-direction.y, direction.x);
  auto head_left = head_base + perpendicular.Scale(head_width);
  auto head_right = head_base - perpendicular.Scale(head_width);

  // Draw the triangular arrowhead
  triangle_buffer.emplace_back(head_left, tip, head_right, color);
}

void ProbeRenderer::flush() {
  for (const auto &line : line_buffer) {
    line.color.DrawLine(line.start, line.end);
  }

  line_buffer.clear();

  for (const auto &triangle : triangle_buffer) {
    DrawTriangle(triangle.a, triangle.b, triangle.c, triangle.color);
  }

  triangle_buffer.clear();
}
