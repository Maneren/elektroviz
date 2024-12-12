#include "Grid.hpp"
#include "Camera2D.hpp"
#include "Position.hpp"
#include "Probe.hpp"
#include "defs.hpp"
#include "parallel.hpp"
#include <Color.hpp>
#include <Vector2.hpp>
#include <algorithm>
#include <memory>
#include <span>
#include <vector>

void GridLine::draw() const { start.DrawLine(end, color); }

std::vector<GridLine> generateLines(
    const raylib::Vector2 size,
    const raylib::Vector2 spacing,
    const raylib::Color color,
    const raylib::Camera2D &camera
) {
  const auto size_half = size / 2.0f;
  const auto screen_to_world = [&](const float x, const float y) {
    return camera.GetScreenToWorld({x, y});
  };

  std::vector<GridLine> lines;

  for (float y = size_half.y; y <= size.y; y += spacing.y) {
    lines.emplace_back(
        screen_to_world(0.f, y), screen_to_world(size.x, y), color
    );
    lines.emplace_back(
        screen_to_world(0.f, size.y - y),
        screen_to_world(size.x, size.y - y),
        color
    );
  }

  for (float x = size_half.x; x <= size.x; x += spacing.x) {
    lines.emplace_back(
        screen_to_world(x, 0.f), screen_to_world(x, size.y), color
    );
    lines.emplace_back(
        screen_to_world(size.x - x, 0.f),
        screen_to_world(size.x - x, size.y),
        color
    );
  }

  return lines;
}

std::vector<Probe> generateProbes(
    const raylib::Vector2 size,
    const raylib::Vector2 spacing,
    const raylib::Color color,
    const raylib::Camera2D &camera
) {
  const auto size_half = size / 2.0f;
  const auto screen_to_world = [&](const Vector2 position) {
    return camera.GetScreenToWorld(position);
  };

  std::vector<Probe> probes;

  for (float y = 0.f; y <= size_half.y + spacing.y; y += spacing.y) {
    for (float x = 0.f; x <= size_half.x + spacing.x; x += spacing.x) {
      for (const auto pos :
           std::vector<Vector2>{{x, y}, {x, -y}, {-x, y}, {-x, -y}}) {
        auto position = size_half + pos;
        probes.emplace_back(
            std::make_unique<position::Static>(screen_to_world(position)),
            color,
            0,
            std::min(spacing.x, spacing.y) / camera.GetZoom()
        );
      }
    }
  }

  return probes;
}

void Grid::resize(
    const raylib::Vector2 size,
    const raylib::Vector2 spacing,
    const raylib::Camera2D &camera
) {
  lines = generateLines(size, spacing, line_color, camera);
  probes = generateProbes(size, spacing, probe_color, camera);
}

void Grid::draw() const {
  ProbeRenderer renderer{};

  for (auto &probe : probes) {
    renderer.draw_to_buffer(probe);
  }

  renderer.flush();

  for (auto &line : lines) {
    line.draw();
  }
}

void Grid::update(
    const float timeDelta,
    const double elapsedTime,
    const std::span<Charge> &charges
) {
  for (auto &probe : probes) {
    probe.update(timeDelta, elapsedTime, charges);
  }
}
