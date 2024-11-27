#pragma once
#include <Color.hpp>
#include <Vector2.hpp>
#include <format>
#include <numbers>

// Initial screen dimensions, irrelevant after first resize
constexpr int SCREEN_WIDTH = 1000;
constexpr int SCREEN_HEIGHT = 800;

constexpr float EPSILON_0 = 8.8541878128e-12f;
constexpr float K_E = 1 / (4 * std::numbers::pi_v<float> * EPSILON_0);

constexpr int FONT_SIZE = 22;
constexpr int FONT_SIZE_SMALL = 18;
constexpr int FONT_SIZE_TINY = 8;
constexpr float GLOBAL_SCALE = 100.0f;

constexpr int LINES_PER_CHARGE = 12;

constexpr int BACKGROUND_SUBSAMPLING = 2;

template <> struct std::formatter<raylib::Vector2> {
  constexpr auto parse(std::format_parse_context const &ctx) const {
    return ctx.begin();
  }
  template <typename FormatContext>
  auto format(const raylib::Vector2 &c, FormatContext &ctx) const {
    return std::format_to(ctx.out(), "({}, {})", c.x, c.y);
  }
};

template <> struct std::formatter<raylib::Color> {
  constexpr auto parse(std::format_parse_context const &ctx) const {
    return ctx.begin();
  }
  template <typename FormatContext>
  auto format(const raylib::Color &c, FormatContext &ctx) const {
    return std::format_to(ctx.out(), "({}, {}, {}, {})", c.r, c.g, c.b, c.a);
  }
};
