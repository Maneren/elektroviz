#include "Plot.hpp"
#include "Color.hpp"
#include "Vector2.hpp"
#include "defs.hpp"
#include "raylib.h"
#include <algorithm>
#include <format>
#include <ranges>
#include <vector>

namespace views = std::views;
namespace ranges = std::ranges;

float row_maxes(auto &data) {
  return ranges::max(data | views::transform([](auto &row) {
                       auto row_data = std::get<1>(row);
                       if (row_data.empty())
                         return 0.f;
                       auto [min_it, max_row] = ranges::minmax(row_data);
                       return std::max(std::abs(min_it), max_row);
                     }));
}

void Plot::draw(const std::vector<raylib::Color> &probe_colors) const {
  auto border = raylib::Vector2{2.f, 2.f};
  accent_color.DrawRectangle(position, size);
  background_color.DrawRectangle(position + border, size - border * 2.f);

  auto inner_size = size - raylib::Vector2{3.f * border.x, border.y};
  auto graph_size =
      inner_size - raylib::Vector2{
                       static_cast<float>(FONT_SIZE_SMALL) + border.x,
                       static_cast<float>(2 * FONT_SIZE_SMALL)
                   };

  auto top_edge = position.y + border.y;
  auto left_edge = position.x + border.x;
  auto inner_left_edge = left_edge + border.x + FONT_SIZE_SMALL;
  auto right_edge = position.x + size.x - border.x;
  auto bottom_edge = top_edge + inner_size.y;
  auto vertical_midpoint = (top_edge + bottom_edge) / 2.f;
  auto horizontal_midpoint = (inner_left_edge + right_edge) / 2.f;

  accent_color.DrawLine(
      {inner_left_edge, vertical_midpoint}, {right_edge, vertical_midpoint}, 2
  );
  DrawTriangle(
      {right_edge + border.x, vertical_midpoint},
      {right_edge - 10.f, vertical_midpoint - 5.f},
      {right_edge - 10.f, vertical_midpoint + 5.f},
      accent_color
  );
  accent_color.DrawLine(
      {inner_left_edge, vertical_midpoint / 2.f},
      {right_edge, vertical_midpoint / 2.f}
  );
  accent_color.DrawLine(
      {inner_left_edge, vertical_midpoint * 1.5f},
      {right_edge, vertical_midpoint * 1.5f}
  );

  accent_color.DrawLine(
      {inner_left_edge, top_edge}, {inner_left_edge, bottom_edge}, 2
  );
  DrawTriangle(
      {inner_left_edge, top_edge - border.y},
      {inner_left_edge - 5.f, top_edge + 10.f},
      {inner_left_edge + 5.f, top_edge + 10.f},
      accent_color
  );

  // auto row_data = std::get<1>(row),
  //      [ min_it, max_row ] = ranges::minmax(row_data);

  float max = row_maxes(data);

  auto display_max = max * K_E;

  auto y_max_value_text = std::format(" {:.2g}", display_max);
  raylib::DrawText(
      y_max_value_text,
      inner_left_edge + 5.f,
      top_edge + 5.f,
      FONT_SIZE_SMALL,
      accent_color
  );

  if (GetScreenHeight() > 1000) {
    auto y_mid_value_text = std::format("{:.2g}", display_max / 2.f);
    raylib::DrawText(
        y_mid_value_text,
        inner_left_edge + 5.f,
        vertical_midpoint / 2.f + 1.f,
        FONT_SIZE_SMALL,
        accent_color
    );
  }

  auto y_zero_text = std::string{"0"};
  raylib::DrawText(
      y_zero_text,
      inner_left_edge + 5.f,
      vertical_midpoint + 2.f,
      FONT_SIZE_SMALL,
      accent_color
  );

  if (GetScreenHeight() > 1000) {
    auto y_mid_value_text2 = std::format("-{:.2g}", display_max / 2.f);
    raylib::DrawText(
        y_mid_value_text2,
        inner_left_edge + 5.f,
        vertical_midpoint * 1.5f + 1.f,
        FONT_SIZE_SMALL,
        accent_color
    );
  }

  auto y_min_value_text = std::format("-{:.2g}", display_max);
  raylib::DrawText(
      y_min_value_text,
      inner_left_edge + 5.f,
      bottom_edge - FONT_SIZE_SMALL,
      FONT_SIZE_SMALL,
      accent_color
  );

  auto x_axis_label = std::string{"time [s]"};
  raylib::DrawText(
      x_axis_label,
      horizontal_midpoint - raylib::MeasureText(x_axis_label, FONT_SIZE_SMALL),
      bottom_edge - FONT_SIZE_SMALL,
      FONT_SIZE_SMALL,
      accent_color
  );

  auto y_axis_label = std::string{"E [N/C]"};
  raylib::DrawTextPro(
      Font(),
      y_axis_label,
      {left_edge + 1.f, vertical_midpoint},
      {raylib::MeasureText(y_axis_label, FONT_SIZE_SMALL) / 2.f, 0.f},
      -90.f,
      FONT_SIZE_SMALL,
      1.f,
      accent_color
  );

  // draw small vertical lines seperating thirds with te4xt denoting that
  // they represent 1/3 of the total time
  for (int i = 0; i < 3; i++) {
    auto x = inner_left_edge + graph_size.x / 3.f * (i + 1);
    accent_color.DrawLine(x, top_edge, x, bottom_edge);
    auto text = std::format("{}", (i + 1) * 10);
    auto text_width = raylib::MeasureText(text, FONT_SIZE_SMALL);
    raylib::DrawText(
        text,
        x - text_width - 5.f,
        bottom_edge - FONT_SIZE_SMALL,
        FONT_SIZE_SMALL,
        accent_color
    );
  }

  std::vector<raylib::Vector2> draw_buffer;
  draw_buffer.reserve(resolution);

  float resolution = static_cast<float>(this->resolution);
  // for each row in data
  for (auto [row, color] : ranges::zip_view(data, probe_colors)) {
    auto &[offset, row_data] = row;
    // start at right edge and add points to the buffer
    for (auto [i, value] :
         row_data | views::as_const | views::reverse | views::enumerate) {
      auto draw_value = graph_size.y * value / max / 2.f;
      auto x_offset = graph_size.x * (i + offset) / resolution;
      draw_buffer.emplace_back(
          right_edge - x_offset, vertical_midpoint - draw_value
      );
    }

    color.DrawLineStrip(draw_buffer.data(), draw_buffer.size());
    draw_buffer.clear();
  }
}

// TODO: fix timing
void Plot::update(
    const float,
    const double elapsedTime,
    const float speed,
    const std::vector<std::optional<Probe>> &probes
) {
  while (data.size() < probes.size())
    data.emplace_back(0, std::deque<float>());

  // sample at most 60 times per second of simulation
  if (elapsedTime - last_update <= 1.0 / 61.0 / speed)
    return;

  last_update = elapsedTime;

  for (auto [row, probe] : ranges::zip_view(data, probes)) {
    auto &[offset, row_data] = row;
    if (probe == std::nullopt && offset < static_cast<int>(resolution)) {
      offset += speed;
    } else {
      for (int i = 0; i < speed; i++) {
        row_data.push_back(probe->sample_potencial());
      }
    }

    while (row_data.size() + offset >= resolution && !row_data.empty()) {
      row_data.pop_front();
    }
  }
}
