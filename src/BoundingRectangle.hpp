#pragma once
#include <Vector2.hpp>

class BoundingRectangle {
public:
  BoundingRectangle(raylib::Vector2 position, raylib::Vector2 size)
      : position(position), size(size) {}

  BoundingRectangle operator+(const BoundingRectangle &other) const;
  void operator+=(const BoundingRectangle &other) {
    auto res = *this + other;
    this->position = res.position;
    this->size = res.size;
  }

  raylib::Vector2 position;
  raylib::Vector2 size;
};
