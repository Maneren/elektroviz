#pragma once
#include <Vector2.hpp>

class BoundingSquare {
public:
  BoundingSquare(raylib::Vector2 position, raylib::Vector2 size)
      : position(position), size(size) {}

  BoundingSquare operator+(const BoundingSquare &other) const;
  void operator+=(const BoundingSquare &other) {
    auto res = *this + other;
    this->position = res.position;
    this->size = res.size;
  }

  raylib::Vector2 position;
  raylib::Vector2 size;
};
