#include "Charge.hpp"
#include "MathEval.hpp"
#include "defs.hpp"
#include "utils.hpp"
#include <raymath.h>

float VariableChargeStrength::operator()(float elapsed) const {
  auto vars = variables;
  vars["t"] = elapsed;
  return evaluate(func, vars);
}

void Charge::update(float timeDelta) { _strength = strengthFn(timeDelta); }

void Charge::draw() const {
  auto color = lerp(NEGATIVE, POSITIVE, Normalize(_strength, -4, 4));
  (_position * GLOBAL_SCALE).DrawCircle(10 * std::abs(_strength), color);
}

const raylib::Color Charge::POSITIVE = raylib::Color(255, 0, 0, 255);
const raylib::Color Charge::NEGATIVE = raylib::Color(0, 0, 255, 255);
