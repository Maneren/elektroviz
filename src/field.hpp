#pragma once
#include "Charge.hpp"
#include "Vector2.hpp"
#include <vector>

raylib::Vector2 EFieldAt(const raylib::Vector2 point,
                         const std::vector<Charge> &charges);

float eFieldAt(const raylib::Vector2 point, const std::vector<Charge> &charges);
