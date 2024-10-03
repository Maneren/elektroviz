#pragma once
#include "Charge.hpp"
#include "Vector2.hpp"
#include <vector>

raylib::Vector2 eFieldAt(const raylib::Vector2 point,
                         const std::vector<Charge> &charges);
