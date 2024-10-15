#include "Charge.hpp"
#include "MathEval.hpp"
#include "defs.hpp"
#include "utils.hpp"
#include <algorithm>

float charge::VariableStrength::operator()(const float elapsed) const {
  auto vars = variables;
  vars["t"] = elapsed;
  return evaluate(func, vars);
}

void Charge::update([[maybe_unused]] const float timeDelta,
                    const float elapsedTime) {
  _strength = (*strengthFn)(elapsedTime);
}

void Charge::draw() const {
  auto radius = std::sqrt(std::abs(_strength)) * 12;
  // outline
  (_position * GLOBAL_SCALE).DrawCircle(radius + 1, raylib::Color::Black());

  auto normalized = std::clamp(_strength / 5, -1.f, 1.f);

  auto color =
      (normalized > 0
           ? lerpColor(raylib::Color::RayWhite(), POSITIVE, normalized)
           : lerpColor(raylib::Color::RayWhite(), NEGATIVE, -normalized));

  (_position * GLOBAL_SCALE).DrawCircle(radius, color);
}

raylib::Vector2 Charge::E(const raylib::Vector2 &point) const {
  auto direction = (point - _position);
  auto distance = direction.Length();
  return direction * _strength / (distance * distance * distance);
}

float Charge::potential(const raylib::Vector2 &point) const {
  auto distanceSqr = (point - _position).LengthSqr();
  return _strength / distanceSqr;
}

const raylib::Color Charge::POSITIVE = raylib::Color(255, 0, 0, 255);
const raylib::Color Charge::NEGATIVE = raylib::Color(0, 0, 255, 255);
