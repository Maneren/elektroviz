#pragma once
#include <format>
#include <functional>
#include <raylib-cpp.hpp>

class ChargeStrength {
public:
  virtual float operator()(float timeDelta) const = 0;
};

class ConstantChargeStrength : public ChargeStrength {
public:
  ConstantChargeStrength(const float strength) : strength(strength) {}
  float operator()([[maybe_unused]] float elapsed) const { return strength; }
  const float strength;
};

class VariableChargeStrength : public ChargeStrength {
public:
  VariableChargeStrength(const std::string &label,
                         const std::function<float(float)> func)
      : label(label), func(func) {}
  float operator()(float elapsed) const { return func(elapsed); }
  const std::string &label;
  const std::function<float(float)> func;
};

template <> struct std::formatter<VariableChargeStrength> {
  template <typename FormatContext>
  auto format(VariableChargeStrength const &c, FormatContext &ctx) {
    return std::format_to(ctx.out(), "Variable({})", c.label);
  }
};

template <> struct std::formatter<ConstantChargeStrength> {
  template <typename FormatContext>
  auto format(ConstantChargeStrength const &c, FormatContext &ctx) {
    return std::format_to(ctx.out(), "Constant({})", c.strength);
  }
};

class Charge {
public:
  Charge(const raylib::Vector2 &position, const ChargeStrength &strength)
      : _position(position), strengthFn(strength) {};
  void update(float elapsed);
  void draw() const;

  raylib::Vector2 position() const { return _position; }
  float strength() const { return _strength; }

private:
  static const raylib::Color POSITIVE;
  static const raylib::Color NEGATIVE;
  raylib::Vector2 _position;
  const ChargeStrength &strengthFn;
  float _strength;

  friend struct std::formatter<Charge>;
};

template <> struct std::formatter<Charge> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(Charge const &c, FormatContext &ctx) {
    return std::format_to(ctx.out(), "Charge({}, {} ({}))", c.position(),
                          c.strengthFn, c.strength());
  }
};
