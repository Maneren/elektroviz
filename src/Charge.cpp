#include "Charge.hpp"
#include "Color.hpp"
#include "MathEval.hpp"
#include "Vector2.hpp"
#include "defs.hpp"
#include "utils.hpp"
#include <cmath>
#include <unordered_map>

float charge::VariableStrength::operator()(const double elapsed) const {
  auto vars = variables;
  vars["t"] = elapsed;
  return evaluate(func, vars);
}

void Charge::update(
    [[maybe_unused]] const float timeDelta, const double elapsedTime
) {
  _strength = (*strengthFn)(elapsedTime);
}

void Charge::draw() const {
  auto radius = draw_radius();
  auto screen_position = _position;

  // outline
  screen_position.DrawCircle(radius, raylib::Color::RayWhite());

  auto color = lerpColor3(
      NEGATIVE, raylib::Color::LightGray(), POSITIVE, sigmoid(_strength * 2.f)
  );

  // fill
  screen_position.DrawCircle(radius - 1, color);
}

BoundingRectangle Charge::bounding_square() const {
  raylib::Vector2 half_size{50, 50};
  return {_position - half_size, half_size * 2.f};
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

bool Charge::contains(const raylib::Vector2 &point) const {
  auto radius = draw_radius();
  return (point - _position).LengthSqr() < radius * radius;
}

const raylib::Color Charge::POSITIVE = raylib::Color(255, 0, 0, 255);
const raylib::Color Charge::NEGATIVE = raylib::Color(40, 40, 255, 255);
