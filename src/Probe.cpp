#include "Probe.hpp"
#include "field.hpp"

void Probe::update(const float timeDelta, const double elapsedTime,
                   const std::vector<Charge> &charges) {
  _position->update(timeDelta, elapsedTime);
  _sample = field::E((*_position)(), charges);
}
