#pragma once
#include "Charge.hpp"
#include <Vector2.hpp>
#include <span>

namespace field {

raylib::Vector2
E(const raylib::Vector2 point, const std::span<const Charge> &charges);

float potential(
    const raylib::Vector2 point, const std::span<const Charge> &charges
);

} // namespace field
