#pragma once
#include <format>
#include <memory>
#include <raylib-cpp.hpp>

class ChargeStrength {
public:
  virtual float operator()(const float timeDelta) const = 0;
  virtual ~ChargeStrength() = default;
};

class ConstantChargeStrength : public ChargeStrength {
public:
  ConstantChargeStrength(const float strength) : strength(strength) {}
  float operator()([[maybe_unused]] const float elapsed) const {
    return strength;
  }
  const float strength;

};

class VariableChargeStrength : public ChargeStrength {
public:
  VariableChargeStrength(const std::string &func) : func(func) {
    std::println("VariableChargeStrength constructor func: {}", this->func);
  }
  float operator()(const float elapsed) const;
  const std::string func;

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
  Charge(const Charge &) = delete;
  Charge(const raylib::Vector2 &position,
         std::unique_ptr<ChargeStrength> strength)
      : _position(position), strengthFn(std::move(strength)) {};

  Charge(Charge &&) = default;
  Charge &operator=(Charge &&) = default;

  void update(const float elapsed);
  void draw() const;

  raylib::Vector2 position() const { return _position; }
  float strength() const { return _strength; }

private:
  static const raylib::Color POSITIVE;
  static const raylib::Color NEGATIVE;
  raylib::Vector2 _position;
  std::unique_ptr<ChargeStrength> strengthFn;
  float _strength;
  friend struct std::formatter<Charge>;
};

template <> struct std::formatter<Charge> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(Charge const &c, FormatContext &ctx) const {
    return std::format_to(ctx.out(), "Charge({}, {} ({}))", c._position,
                          *c.strengthFn, c._strength);
  }
};
