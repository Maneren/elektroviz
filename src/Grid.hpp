#pragma once
#include "Charge.hpp"
#include "Probe.hpp"
#include <raylib-cpp.hpp>

class GridLine {
public:
  GridLine(const raylib::Vector2 start, const raylib::Vector2 end,
           raylib::Color color = raylib::Color::Gray())
      : start(start), end(end), color(color) {};
  void draw() const;

private:
  raylib::Vector2 start;
  raylib::Vector2 end;
  raylib::Color color;

  friend struct std::formatter<GridLine>;
};

template <> struct std::formatter<GridLine> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const GridLine &p, FormatContext &ctx) const {
    return std::format_to(ctx.out(), "GridLine(start: {}, end: {}, color: {})",
                          p.start, p.end, p.color);
  }
};

std::vector<GridLine> generateLines(const raylib::Vector2 size,
                                    const raylib::Vector2 spacing,
                                    const raylib::Color color);

std::vector<Probe> generateProbes(const raylib::Vector2 size,
                                  const raylib::Vector2 spacing,
                                  const raylib::Color color);

class Grid {
public:
  Grid(const raylib::Vector2 size, const raylib::Vector2 spacing,
       const raylib::Color line_color = raylib::Color::LightGray(),
       const raylib::Color probe_color = raylib::Color::Gray())
      : line_color(line_color), probe_color(probe_color),
        origin(raylib::Vector2{0, 0}),
        lines(generateLines(size, spacing, line_color)),
        probes(generateProbes(size, spacing, probe_color)) {};
  void draw() const;
  void update(const float timeDelta, const float elapsedTime,
              const std::vector<Charge> &charges);
  void resize(const raylib::Vector2 size, const raylib::Vector2 spacing);

private:
  const raylib::Color line_color;
  const raylib::Color probe_color;
  raylib::Vector2 origin;
  std::vector<GridLine> lines;
  std::vector<Probe> probes;
};
