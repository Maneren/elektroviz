#include "Grid.hpp"
#include <vector>

void GridLine::draw() const { start.DrawLine(end, color); }

std::vector<GridLine> generateLines(raylib::Vector2 size,
                                    raylib::Vector2 spacing,
                                    raylib::Color color) {
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

void Grid::resize(raylib::Vector2 size, const raylib::Vector2 spacing) {
  lines = generateLines(size, spacing, color);
}

void Grid::draw() const {
  for (auto &line : lines) {
    line.draw();
  }
}

void Grid::update(float timeDelta, std::vector<Charge> &charges) {}
