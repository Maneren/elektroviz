#include "Charge.hpp"
#include "MathEval.hpp"
#include "defs.hpp"
#include "utils.hpp"
#include <Color.hpp>
#include <Functions.hpp>
#include <Vector2.hpp>
#include <cmath>
#include <unordered_map>

float charge::VariableStrength::operator()(const double elapsed) const {
  const std::unordered_map<std::string, float> variables = {
      {"t", elapsed},
      {"pi", std::numbers::pi_v<float>},
      {"e", std::numbers::e_v<float>}
  };
  return evaluate(func, variables);
}

void Charge::update(
    [[maybe_unused]] const float timeDelta, const double elapsedTime
) {
  _strength = (*strengthFn)(elapsedTime)*_strengthModifier;
}

void Charge::draw() const {
  auto radius = draw_radius();
  auto screen_position = _position;

  // outline
  screen_position.DrawCircle(radius, raylib::Color::RayWhite());

  auto color = lerpColor3(
      NEGATIVE, raylib::Color::Blank(), POSITIVE, sigmoid(_strength * 4.f)
  );

  // fill
  screen_position.DrawCircle(radius - 1, color);

  // text
  auto msg = std::format("{:.2G} C", _strength);
  int width = raylib::MeasureText(msg, FONT_SIZE_TINY);
  raylib::DrawText(
      msg,
      screen_position.x - width / 2.f,
      screen_position.y - static_cast<float>(FONT_SIZE_TINY) / 2,
      FONT_SIZE_TINY,
      raylib::Color::RayWhite()
  );
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
const raylib::Color Charge::NEGATIVE = raylib::Color(50, 50, 255, 255);
