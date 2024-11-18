#include "Plot.hpp"
#include "Color.hpp"
#include "defs.hpp"
#include <algorithm>
#include <format>
#include <ranges>
#include <vector>

namespace views = std::views;
namespace ranges = std::ranges;

void Plot::draw(const std::vector<raylib::Color> &probe_colors) const {
  background_color.DrawRectangle(position, size);

  auto vertical_midpoint = position.y + size.y / 2.f;
  auto right_edge = position.x + size.x;

  raylib::Color::Black().DrawLine(
      {position.x, vertical_midpoint}, {right_edge, vertical_midpoint}
  );

  auto row_maxes = data | views::transform([](auto &row) {
                     if (std::get<1>(row).empty())
                       return 0.f;
                     auto [min_it, max_row] = ranges::minmax(std::get<1>(row));
                     return std::max(std::abs(min_it), max_row);
                   });
  float max = ranges::max(row_maxes) * 3.f;

  auto display_max = max * K_E;

  auto top_text = std::format(" {:.2g} N/C", display_max);
  raylib::DrawText(
      top_text, position.x + 5.f, position.y, FONT_SIZE, raylib::Color::Black()
  );

  auto middle_text = std::string{" 0"};
  raylib::DrawText(
      middle_text,
      position.x + 5.f,
      vertical_midpoint,
      FONT_SIZE,
      raylib::Color::Black()
  );

  auto bottom_text = std::format("-{:.2g} N/C", display_max);
  raylib::DrawText(
      bottom_text,
      position.x + 5.f,
      position.y + size.y - FONT_SIZE,
      FONT_SIZE,
      raylib::Color::Black()
  );

  auto right_text = std::string{"30 s"};
  raylib::DrawText(
      right_text,
      right_edge - 5.f - raylib::MeasureText(right_text, FONT_SIZE),
      position.y + size.y - FONT_SIZE,
      FONT_SIZE,
      raylib::Color::Black()
  );

  std::vector<raylib::Vector2> draw_buffer;
  draw_buffer.reserve(resolution);

  float resolution = static_cast<float>(this->resolution);
  // for each row in data
  for (auto [row, color] : ranges::zip_view(data, probe_colors)) {
    auto &[offset, row_data] = row;
    // start at right edge and add points to the buffer
    for (auto [i, value] :
         row_data | views::as_const | views::reverse | views::enumerate) {
      auto draw_value = size.y * value / max;
      auto x_offset = size.x * (i + offset) / resolution;
      draw_buffer.emplace_back(
          right_edge - x_offset, vertical_midpoint - draw_value
      );
    }

    color.DrawLineStrip(draw_buffer.data(), draw_buffer.size());
    draw_buffer.clear();
  }
}

void Plot::update(
    const float,
    const double elapsedTime,
    const std::vector<std::optional<Probe>> &probes
) {
  while (data.size() < probes.size())
    data.emplace_back(0, std::deque<float>());

  // sample at most 60 times per second
  if (elapsedTime - last_update <= 1.0 / 60.0)
    return;

  last_update = elapsedTime;

  for (auto [row, probe] : ranges::zip_view(data, probes)) {
    auto &[offset, row_data] = row;
    if (probe == std::nullopt && offset < resolution) {
      offset++;
    } else {
      row_data.push_back(probe->sample_potencial());
    }

    if (row_data.size() + offset >= resolution && !row_data.empty()) {
      row_data.pop_front();
    }
  }
}
