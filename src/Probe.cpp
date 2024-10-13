#include "Probe.hpp"
#include "field.hpp"

void Probe::update(const float timeDelta, const float elapsedTime,
                   const std::vector<Charge> &charges) {
  _position->update(timeDelta, elapsedTime);
  _sample = E((*_position)(), charges);
}
