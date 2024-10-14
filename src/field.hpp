#pragma once
#include "Charge.hpp"
#include <Vector2.hpp>
#include <vector>

namespace field {

raylib::Vector2 E(const raylib::Vector2 point,
                  const std::vector<Charge> &charges);

float potential(const raylib::Vector2 point,
                const std::vector<Charge> &charges);

} // namespace field
