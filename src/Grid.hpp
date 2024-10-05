#pragma once
#include "Charge.hpp"
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

class Grid {
public:
  Grid(const raylib::Vector2 size, const float spacing);
  void draw() const;
  void update(float timeDelta, std::vector<Charge> &charges);
  void resize(raylib::Vector2 size);

private:
  float spacing;
  raylib::Vector2 origin;
  std::vector<GridLine> lines;
};
