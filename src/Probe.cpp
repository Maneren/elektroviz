#include "Probe.hpp"
#include "Charge.hpp"
#include "Vector2.hpp"
#include "field.hpp"
#include <span>

void Probe::update(
    const float timeDelta,
    const double elapsedTime,
    const std::span<Charge> &charges
) {
  _position->update(timeDelta, elapsedTime);
  _sample = field::E((*_position)(), charges);
  _sample_potencial = field::potential((*_position)(), charges);
}
