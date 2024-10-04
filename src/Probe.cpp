#include "Probe.hpp"
#include "field.hpp"

void Probe::update(const std::vector<Charge> &charges) {
  _sample = eFieldAt(position, charges).Length();
}
