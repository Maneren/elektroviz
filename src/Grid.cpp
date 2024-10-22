#include "Grid.hpp"
#include "Position.hpp"
#include "Probe.hpp"
#include "defs.hpp"
#include "parallel.hpp"
#include <Color.hpp>
#include <Vector2.hpp>
#include <algorithm>
#include <functional>
#include <memory>
#include <span>
#include <utility>
#include <vector>

void GridLine::draw() const { start.DrawLine(end, color); }

std::vector<GridLine> generateLines(const raylib::Vector2 size,
                                    const raylib::Vector2 spacing,
                                    const raylib::Color color) {
  const auto size_half = size / 2.0f;

  std::vector<GridLine> lines;

  for (float y = 0; y <= size_half.y; y += spacing.y) {
    lines.push_back(GridLine({-size_half.x, y}, {size_half.x, y}, color));
    lines.push_back(GridLine({-size_half.x, -y}, {size_half.x, -y}, color));
  }

  for (float x = 0; x <= size_half.x; x += spacing.x) {
    lines.push_back(GridLine({x, -size_half.y}, {x, size_half.y}));
    lines.push_back(GridLine({-x, -size_half.y}, {-x, size_half.y}));
  }

  return lines;
}

std::vector<Probe> generateProbes(const raylib::Vector2 size,
                                  const raylib::Vector2 spacing,
                                  const raylib::Color color) {
  const auto world_size_half = screen_to_world(size);
  const auto world_spacing = screen_to_world(spacing);

  std::vector<Probe> probes;

  for (float y = 0; y <= world_size_half.y + world_spacing.y;
       y += world_spacing.y) {
    for (float x = 0; x <= world_size_half.x + world_spacing.x;
         x += world_spacing.x) {
      for (raylib::Vector2 pos :
           std::vector<raylib::Vector2>{{x, y}, {x, -y}, {-x, y}, {-x, -y}}) {
        Probe probe{std::make_unique<position::Static>(pos), color, 0};
        probe.scale = std::min(spacing.x, spacing.y) / 30;
        probes.push_back(std::move(probe));
      }
    }
  }

  return probes;
}

void Grid::resize(const raylib::Vector2 size, const raylib::Vector2 spacing) {
  lines = generateLines(size, spacing, line_color);
  probes = generateProbes(size, spacing, probe_color);
}

void Grid::draw() const {
  for (auto &line : lines) {
    line.draw();
  }

  for (auto &probe : probes) {
    probe.draw<true>();
  }
}

void Grid::update(const float timeDelta, const double elapsedTime,
                  const std::span<Charge> &charges) {
  for (auto &probe : probes) {
    probe.update(timeDelta, elapsedTime, charges);
  }
}
