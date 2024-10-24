#pragma once
#include <Color.hpp>
#include <string>

/**
 * @brief Linearly interpolate between two colors (optimized interface).
 *
 * @param a first color (0)
 * @param b second color (1000)
 * @param t interpolation factor (0..1000)
 * @return Interpolated color
 */
constexpr raylib::Color lerpColor(const Color &a, const Color &b, int t) {
  return {static_cast<unsigned char>(a.r + t * (b.r - a.r) / 1000),
          static_cast<unsigned char>(a.g + t * (b.g - a.g) / 1000),
          static_cast<unsigned char>(a.b + t * (b.b - a.b) / 1000)};
}

/**
 * @brief Linearly interpolate between two colors
 *
 * @param a first color (0.0)
 * @param b second color (1.0)
 * @param t interpolation factor (0.0..1.0)
 * @return Interpolated color
 *
 * Uses `lerpColor(Color, Color, int)` intenally, converting `t` to `int`.
 */
constexpr raylib::Color lerpColor(const Color &a, const Color &b, float t) {
  return lerpColor(a, b, static_cast<int>(t * 1000.f));
}

/**
 * @brief Interpolate between three colors
 *
 * @param a first color (-1.0)
 * @param b middle color (0.0)
 * @param c last color (1.0)
 * @param t interpolation factor (-1.0..1.0)
 * @return Interpolated color
 *
 * Uses `lerpColor(Color, Color, int)` intenally, converting `t` to `int`, for
 * performance.
 */
constexpr raylib::Color lerpColor3(const Color &a, const Color &b,
                                   const Color &c, float t) {
  int t_int = static_cast<int>(t * 1000.f);

  if (t_int <= 0) {
    return lerpColor(a, b, t_int + 1000);
  } else {
    return lerpColor(b, c, t_int);
  }
}

constexpr float sigmoid(float x) { return x / (1.f + std::abs(x)); }

std::string trim(const std::string &str);
