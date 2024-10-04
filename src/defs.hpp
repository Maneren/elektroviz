#pragma once
#include "raylib.h"
#include <format>
#include <raylib-cpp.hpp>

using uint = unsigned int;

// Global screen dimensions, updated on window resize
static uint SCREEN_WIDTH = 800;
static uint SCREEN_HEIGHT = 600;

constexpr float EPSILON_0 = 8.8541878128e-12;
constexpr float K_E = 1 / (4 * PI * EPSILON_0);

constexpr float GLOBAL_SCALE = 120.0f;
constexpr float FONT_SIZE = 24.0f;

template <> struct std::formatter<raylib::Vector2> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(raylib::Vector2 const &c, FormatContext &ctx) const {
    return std::format_to(ctx.out(), "({}, {})", c.x, c.y);
  }
};
