#include "Charge.hpp"
#include "MathEval.hpp"
#include "defs.hpp"
#include "utils.hpp"
#include <raymath.hpp>

float VariableChargeStrength::operator()(const float elapsed) const {
  auto vars = variables;
  vars["t"] = elapsed;
  return evaluate(func, vars);
}

void Charge::update([[maybe_unused]] const float timeDelta,
                    const float elapsedTime) {
  _strength = (*strengthFn)(elapsedTime);
}

void Charge::draw() const {
  auto color = lerpColor(NEGATIVE, POSITIVE, Normalize(_strength, -4, 4));
  (_position * GLOBAL_SCALE)
      .DrawCircle(2 + 10 * std::abs(_strength), raylib::Color::Black());
  (_position * GLOBAL_SCALE).DrawCircle(10 * std::abs(_strength), color);
}

const raylib::Color Charge::POSITIVE = raylib::Color(255, 0, 0, 255);
const raylib::Color Charge::NEGATIVE = raylib::Color(0, 0, 255, 255);
