#include "Charge.hpp"
#include "MathEval.hpp"
#include "defs.hpp"
#include "utils.hpp"

float charge::VariableStrength::operator()(const double elapsed) const {
  auto vars = variables;
  vars["t"] = elapsed;
  return evaluate(func, vars);
}

void Charge::update([[maybe_unused]] const float timeDelta,
                    const double elapsedTime) {
  _strength = (*strengthFn)(elapsedTime);
}

void Charge::draw() const {
  auto radius = std::sqrt(std::abs(_strength)) * 12;
  // outline
  (_position * GLOBAL_SCALE).DrawCircle(radius + 1, raylib::Color::RayWhite());

  auto color = lerpColor3(NEGATIVE, raylib::Color::DarkGray(), POSITIVE,
                          sigmoid(_strength));

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
const raylib::Color Charge::NEGATIVE = raylib::Color(40, 40, 255, 255);
