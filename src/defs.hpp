#pragma once
#include "Color.hpp"
#include <format>
#include <numbers>
#include <raylib-cpp.hpp>
#include <unordered_map>

// Global screen dimensions, updated on window resize
static size_t SCREEN_WIDTH = 800;
static size_t SCREEN_HEIGHT = 600;

constexpr float EPSILON_0 = 8.8541878128e-12;
constexpr float K_E = 1 / (4 * std::numbers::pi * EPSILON_0);

constexpr float GLOBAL_SCALE = 100.0f;
constexpr float FONT_SIZE = 24.0f;

template <> struct std::formatter<raylib::Vector2> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const raylib::Vector2 &c, FormatContext &ctx) const {
    return std::format_to(ctx.out(), "({}, {})", c.x, c.y);
  }
};

template <> struct std::formatter<raylib::Color> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const raylib::Color &c, FormatContext &ctx) const {
    return std::format_to(ctx.out(), "({}, {}, {}, {})", c.r, c.g, c.b, c.a);
  }
};

static const std::unordered_map<std::string, float> variables = {
    {"t", 0.0f}, {"pi", std::numbers::pi}, {"e", std::numbers::e}};
