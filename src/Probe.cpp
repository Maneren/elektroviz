#include "Probe.hpp"
#include "Charge.hpp"
#include "Vector2.hpp"
#include "defs.hpp"
#include "field.hpp"
#include <span>

void Probe::update(const float timeDelta, const double elapsedTime,
                   const std::span<Charge> &charges) {
  _position->update(timeDelta, elapsedTime);
  _sample = field::E((*_position)(), charges);
}

BoundingSquare Probe::bounding_square() const {
  return _position->bounding_square();
}
