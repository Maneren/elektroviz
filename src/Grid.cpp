#include "Grid.hpp"
#include <vector>

void GridLine::draw() const { start.DrawLine(end, color); }

std::vector<GridLine> generateLines(raylib::Vector2 size, float spacing) {
  const auto size_half = size / 2.0f;

  std::vector<GridLine> lines;

  for (float y = 0; y <= size_half.y; y += spacing) {
    lines.push_back(GridLine({-size_half.x, y}, {size_half.x, y}));
    lines.push_back(GridLine({-size_half.x, -y}, {size_half.x, -y}));
  }

  for (float x = 0; x <= size_half.x; x += spacing) {
    lines.push_back(GridLine({x, -size_half.y}, {x, size_half.y}));
    lines.push_back(GridLine({-x, -size_half.y}, {-x, size_half.y}));
  }

  return lines;
}

Grid::Grid(const raylib::Vector2 size, const float spacing) {
  this->spacing = spacing;
  lines = generateLines(size, spacing);
}

void Grid::resize(raylib::Vector2 size) {
  lines = generateLines(size, spacing);
}

void Grid::draw() const {
  for (auto &line : lines) {
    line.draw();
  }
}

void Grid::update(float timeDelta, std::vector<Charge> &charges) {}
