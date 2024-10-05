#pragma once
#include <format>
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

static const std::unordered_map<std::string, float> variables{{"t", 0.0f},
                                                              {"pi", PI}};

class VariableChargeStrength : public ChargeStrength {
public:
  VariableChargeStrength(const std::string &func) : func(func) {}
  float operator()(float elapsed) const;
  const std::string &func;

};

template <> struct std::formatter<ChargeStrength> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const ChargeStrength &c, FormatContext &ctx) const {
    if (dynamic_cast<const ConstantChargeStrength *>(&c)) {
      return std::format_to(ctx.out(), "{}",
                            dynamic_cast<const ConstantChargeStrength &>(c));
    } else if (dynamic_cast<const VariableChargeStrength *>(&c)) {
      return std::format_to(ctx.out(), "{}",
                            dynamic_cast<const VariableChargeStrength &>(c));
    } else {
      return std::format_to(ctx.out(), "Unknown");
    }
  }
};

template <> struct std::formatter<VariableChargeStrength> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const VariableChargeStrength &c, FormatContext &ctx) const {
    return std::format_to(ctx.out(), "Variable({})", c.func);
  }
};

template <> struct std::formatter<ConstantChargeStrength> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const ConstantChargeStrength &c, FormatContext &ctx) const {
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
  auto format(Charge const &c, FormatContext &ctx) const {
    return std::format_to(ctx.out(), "Charge({}, {} ({}))", c._position,
                          c.strengthFn, c._strength);
  }
};
