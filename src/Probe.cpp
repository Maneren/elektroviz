#include "Probe.hpp"
#include "field.hpp"

void Probe::update(const float timeDelta, const std::vector<Charge> &charges) {
  _position->update(timeDelta);
  _sample = eFieldAt((*_position)(), charges);
}
