#include "Charge.hpp"
#include "utils.hpp"
#include <raymath.h>

void Charge::update(float timeDelta) { _strength = strengthFn(timeDelta); }

void Charge::draw() {
  auto color = lerp(NEGATIVE, POSITIVE, Normalize(_strength, -4, 4));
  _position.DrawCircle(5, color);
}

const raylib::Color Charge::POSITIVE = raylib::Color(255, 0, 0, 255);
const raylib::Color Charge::NEGATIVE = raylib::Color(0, 0, 255, 255);
