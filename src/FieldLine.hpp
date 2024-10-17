#pragma once
#include "Charge.hpp"
#include "field.hpp"
#include <Color.hpp>
#include <Vector2.hpp>
#include <functional>
#include <vector>

class FieldLine {
public:
  FieldLine(const raylib::Vector2 &start_position, const size_t charge_index,
            const raylib::Color &color)
      : color(color), start_position(start_position),
        charge_index(charge_index) {}
  virtual ~FieldLine() = default;
  virtual void update(const std::vector<Charge> &charges);
  void draw() const;

protected:
  virtual raylib::Vector2 field_function(const std::span<const Charge> &charges,
                                         const raylib::Vector2 &point,
                                         const float direction = 1.f) const {
    return field::E(point, charges) * direction;
  };

  virtual bool found_end(const Charge &charge, const raylib::Vector2 &position,
                         const raylib::Vector2 &next_position) const {
    // PERF: this is for some reason a lot faster than Point-Circle check
    return CheckCollisionCircleLine(charge.position(), 0.01f, position,
                                    next_position);
  }

  raylib::Color color;
  raylib::Vector2 start_position;
  size_t charge_index;
  std::vector<Vector2> points{};
};

class EquipotencialLine : public FieldLine {
public:
  EquipotencialLine(const raylib::Vector2 &start_position,
                    const size_t charge_index, const raylib::Color &color)
      : FieldLine(start_position, charge_index, color) {}

private:
  raylib::Vector2 field_function(const std::span<const Charge> &charges,
                                 const raylib::Vector2 &point,
                                 const float direction = 1.f) const override {
    auto sample = field::E(point, charges) * direction;
    return {sample.y, -sample.x};
  };

  bool found_end(const Charge &, const raylib::Vector2 &position,
                 const raylib::Vector2 &next_position) const override {
    return CheckCollisionCircleLine(start_position, 0.01f, position,
                                    next_position);
  }
};

class FieldLines {
  using Line = std::vector<Vector2>;

public:
  FieldLines() = default;
  FieldLines(const size_t lines_per_charge,
             const raylib::Color &color = raylib::Color::White())
      : lines_per_charge(lines_per_charge), color(color) {}

  void update(const std::span<const Charge> &charges);
  void draw() const;

private:
  void draw_line(const Line &lines) const;

  /**
   * @brief Calculate a field line given set of parameters
   *
   * @param charges Reference to span of factors in the field.
   * @param start_point Starting point of the line.
   * @param field_function Function the describes the "direction" of the field
   * in given point.
   * @param end_point_function Return the end point if the line should end
   * already.
   * @param direction Direction of the line relative to the field function.
   * @return the line
   */
  Line calculate_line(
      const raylib::Vector2 &start_point,
      const raylib::Vector2 &start_direction,
      const std::function<raylib::Vector2(raylib::Vector2)> &field_function,
      const std::function<std::optional<raylib::Vector2>(raylib::Vector2)>
          &end_point_function,
      const float direction = 1.f);

  size_t lines_per_charge;
  raylib::Color color;
  std::vector<Line> field_lines{};
  std::vector<Line> equipotencial_lines{};
};
