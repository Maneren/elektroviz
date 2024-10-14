#include "FieldLine.hpp"
#include "defs.hpp"
#include "field.hpp"
#include <algorithm>
#include <functional>
#include <ranges>

namespace field_line {
// Calculate the next value of the function with Euler's method
//
// @param function The function to calculate in form
// $value_{n+1} = value_n + step * function(time, value_n)$
// @param value Current value
// @param time Current time
// @param step Step size
template <typename T>
T Euler(const std::function<T(float, const T &)> &function, const T &value,
        const float &time, const float &step = 0.01f) {
  return value + function(time, value) * step;
}

} // namespace field_line

void FieldLine::update(const float timeDelta, const float elapsedTime,
                       const std::vector<Charge> &charges) {
  constexpr size_t STEPS = 6000;

  auto step = 1e-12f;

  points.clear();
  points.reserve(STEPS);

  const auto field_function =
      [&charges](const float time,
                 const raylib::Vector2 &point) -> raylib::Vector2 {
    return field::E(point, charges);
  };

  auto position = start_position;

  for (const size_t i : std::views::iota(0) | std::views::take(STEPS)) {
    float t = static_cast<float>(i) / STEPS;
    points.push_back(position);

    auto next_position =
        field_line::Euler<raylib::Vector2>(field_function, position, t, step);

    while ((next_position - position).LengthSqr() > 0.1) {
      next_position = (position + next_position) / 2;
    }

    auto collision = std::ranges::any_of(charges, [&](const auto &charge) {
      return CheckCollisionPointLine(charge.position(), position, next_position,
                                     0.5f);
    });

    if (collision)
      break;

    position = next_position;
  }
}

void FieldLine::draw() const {
  for (const auto &[begin, end] :
       points | std::views::transform([](const auto &p) {
         return p * GLOBAL_SCALE;
       }) | std::views::adjacent<2>)
    begin.DrawLine(end, 0.6f, raylib::Color::DarkGreen());
}

// void CalculateEquipotential() {
//   Vector2D<double> startPoint = m_Job.point;
//   Vector2D<double> point = startPoint;
//
//   double dist = 0;
//   double t = 0;
//   unsigned int num_steps = (m_Solver->IsAdaptive() ? 800000 : 1500000);
//   double step = (m_Solver->IsAdaptive() ? 0.001 : 0.0001);
//   double next_step = step;
//
//   fieldLine.AddPoint(startPoint);
//   fieldLine.weightCenter = Vector2D<double>(startPoint);
//
//   for (unsigned int i = 0; i < num_steps; ++i) {
//     point =
//         m_Solver->SolveStep(functorV, point, t, step, next_step, 1E-3,
//         0.01);
//
//     fieldLine.AddPoint(point);
//
//     // 'step' plays the role of the portion of the curve 'weight'
//     fieldLine.weightCenter += point * step;
//
//     t += step;
//     if (m_Solver->IsAdaptive())
//       step = next_step;
//
//     // if the distance is smaller than 6 logical units but the line length
//     is
//     // bigger than double the distance from the start point the code
//     assumes
//     // that the field line closes
//     dist = (startPoint - point).Length();
//     if (dist * distanceUnitLength < 6. && t > 2. * dist) {
//       fieldLine.points.push_back(startPoint); // close the loop
//       fieldLine.weightCenter /= t; // divide by the whole 'weight' of the
//       curve break;
//     }
//   }
// }
