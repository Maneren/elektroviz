#include "BoundingSquare.hpp"
#include "Vector2.hpp"
#include <algorithm>

BoundingSquare BoundingSquare::operator+(const BoundingSquare &other) const {
  raylib::Vector2 new_position{std::min(position.x, other.position.x),
                               std::min(position.y, other.position.y)};

  raylib::Vector2 new_size{
      std::max(position.x + size.x, other.position.x + other.size.x) -
          new_position.x,
      std::max(position.y + size.y, other.position.y + other.size.y) -
          new_position.y};

  return {new_position, new_size};
}
