#pragma once
#include "Charge.hpp"
#include <raylib-cpp.hpp>
#include <vector>

raylib::Vector2 E(const raylib::Vector2 point,
                  const std::vector<Charge> &charges);

float potential(const raylib::Vector2 point,
                const std::vector<Charge> &charges);
