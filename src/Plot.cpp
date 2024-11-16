#include "Plot.hpp"
#include "Color.hpp"
#include "defs.hpp"
#include <algorithm>
#include <format>
#include <ranges>
#include <raylib.h>

namespace views = std::views;
namespace ranges = std::ranges;

void Plot::draw() const {
  background_color.DrawRectangle(position, size);
  auto vertical_midpoint = position.y + size.y / 2.f;
  auto right_edge = position.x + size.x;
  raylib::Color::Black().DrawLine(
      {position.x, vertical_midpoint}, {right_edge, vertical_midpoint}
  );

  auto row_maxes = data | views::transform([](auto &row) {
                     auto [min_it, max_row] = ranges::minmax(row);
                     return std::max(std::abs(min_it), max_row);
                   });
  float max = ranges::max(row_maxes) * 3.f;

  auto display_max = max * K_E;

  auto text_x_pos = [right_edge](auto text) {
    return right_edge - raylib::MeasureText(text, FONT_SIZE);
  };

  auto top_text = std::format(" {:.2g}", display_max);
  raylib::DrawText(
      top_text,
      text_x_pos(top_text),
      position.y,
      FONT_SIZE,
      raylib::Color::Black()
  );

  auto bottom_text = std::format("-{:.2g}", display_max);
  raylib::DrawText(
      bottom_text,
      text_x_pos(bottom_text),
      position.y + size.y - FONT_SIZE,
      FONT_SIZE,
      raylib::Color::Black()
  );

  std::vector<raylib::Vector2> draw_buffer;
  // for each row in data
  for (auto &row : data) {
    // start at right edge and add points to the buffer
    for (auto [i, value] :
         row | views::as_const | views::reverse | views::enumerate) {
      auto draw_value = size.y * value / max;
      auto x_offset = size.x * i / resolution;
      draw_buffer.emplace_back(
          right_edge - x_offset, vertical_midpoint - draw_value
      );
    }

    raylib::Color::Green().DrawLineStrip(
        draw_buffer.data(), draw_buffer.size()
    );
    draw_buffer.clear();
  }
}

void Plot::update(const float, const double elapsedTime, const Probe &probe) {
  if (data.empty())
    data.emplace_back(std::deque<float>{});

  // sample at most 60 times per second
  if (elapsedTime - last_update <= 1.0 / 60) {
    return;
  }

  auto &row = data[0];

  if (row.size() >= resolution) {
    row.pop_front();
  }

  row.push_back(probe.sample_potencial());
  last_update = elapsedTime;
}
