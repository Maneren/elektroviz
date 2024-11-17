#pragma once
#include "BoundingRectangle.hpp"
#include "defs.hpp"
#include <Color.hpp>
#include <Vector2.hpp>
#include <format>
#include <memory>
#include <string>
#include <utility>

namespace charge {

class Strength {
public:
  virtual float operator()(const double elapsed) const = 0;
  virtual ~Strength() = default;
};

class ConstantStrength : public Strength {
public:
  explicit ConstantStrength(const float strength) : strength(strength) {}
  float operator()([[maybe_unused]] const double elapsed) const override {
    return strength;
  }
  const float strength;
};

class VariableStrength : public Strength {
public:
  explicit VariableStrength(const std::string &func) : func(func) {}
  float operator()(const double elapsed) const override;
  const std::string func;
};

} // namespace charge

template <> struct std::formatter<charge::Strength> {
  constexpr auto parse(std::format_parse_context const &ctx) const {
    return ctx.begin();
  }
  template <typename FormatContext>
  auto format(const charge::Strength &c, FormatContext &ctx) const {
    if (dynamic_cast<const charge::ConstantStrength *>(&c)) {
      return std::format_to(
          ctx.out(), "{}", dynamic_cast<const charge::ConstantStrength &>(c)
      );
    } else if (dynamic_cast<const charge::VariableStrength *>(&c)) {
      return std::format_to(
          ctx.out(), "{}", dynamic_cast<const charge::VariableStrength &>(c)
      );
    } else {
      return std::format_to(ctx.out(), "Unknown");
    }
  }
};

template <> struct std::formatter<charge::VariableStrength> {
  constexpr auto parse(std::format_parse_context const &ctx) const {
    return ctx.begin();
  }
  template <typename FormatContext>
  auto format(const charge::VariableStrength &c, FormatContext &ctx) const {
    return std::format_to(ctx.out(), "Variable({})", c.func);
  }
};

template <> struct std::formatter<charge::ConstantStrength> {
  constexpr auto parse(std::format_parse_context const &ctx) const {
    return ctx.begin();
  }
  template <typename FormatContext>
  auto format(const charge::ConstantStrength &c, FormatContext &ctx) const {
    return std::format_to(ctx.out(), "Constant({})", c.strength);
  }
};

class Charge {
public:
  Charge(const Charge &) = delete;
  Charge(
      const raylib::Vector2 &position,
      std::unique_ptr<charge::Strength> strength
  )
      : _position(position), strengthFn(std::move(strength)) {}

  Charge(Charge &&) = default;
  Charge &operator=(Charge &&) = default;

  void update(const float timeDelta, const double elapsedTime);
  void draw() const;

  raylib::Vector2 position() const { return _position; }
  void position(raylib::Vector2 position) { _position = position; }
  float strength() const { return _strength; }

  BoundingRectangle bounding_square() const;

  raylib::Vector2 E(const raylib::Vector2 &point) const;
  float potential(const raylib::Vector2 &point) const;

  bool contains(const raylib::Vector2 &point) const;

  static const raylib::Color POSITIVE;
  static const raylib::Color NEGATIVE;

private:
  float collision_radius() const {
    return (12.f * std::sqrt(std::abs(_strength))) / GLOBAL_SCALE;
  };
  float draw_radius() const { return 12.f * std::sqrt(std::abs(_strength)); };

  raylib::Vector2 _position;
  std::unique_ptr<charge::Strength> strengthFn;
  float _strength;
  friend struct std::formatter<Charge>;
};

template <> struct std::formatter<Charge> {
  constexpr auto parse(std::format_parse_context const &ctx) const {
    return ctx.begin();
  }
  template <typename FormatContext>
  auto format(Charge const &c, FormatContext &ctx) const {
    return std::format_to(
        ctx.out(),
        "Charge({}, {} ({}))",
        c._position,
        *c.strengthFn,
        c._strength
    );
  }
};
