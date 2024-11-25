#pragma once
#include <Color.hpp>
#include <Vector2.hpp>
#include <format>
#include <numbers>
#include <string>
#include <unordered_map>

// Initial screen dimensions, irrelevant after first resize
constexpr int SCREEN_WIDTH = 800;
constexpr int SCREEN_HEIGHT = 600;

constexpr float EPSILON_0 = 8.8541878128e-12f;
constexpr float K_E = 1 / (4 * std::numbers::pi_v<float> * EPSILON_0);

constexpr int FONT_SIZE = 22;
constexpr int FONT_SIZE_SMALL = 18;
constexpr int FONT_SIZE_TINY = 8;
constexpr float GLOBAL_SCALE = 100.0f;

constexpr int LINES_PER_CHARGE = 12;

constexpr int BACKGROUND_SUBSAMPLING = 2;

// TODO: investigate GetScreenToWorld2D from raylib
inline raylib::Vector2
screen_to_world(const raylib::Vector2 &point, float zoom = 1.f) {
  return point / (zoom);
}

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

static const std::unordered_map<std::string, float> variables = {
    {"t", 0.0f},
    {"pi", std::numbers::pi_v<float>},
    {"e", std::numbers::e_v<float>}
};
