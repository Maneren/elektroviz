#include <format>
#include <raylib-cpp.hpp>

class Position {
public:
  virtual raylib::Vector2 operator()() const = 0;
  virtual void update(float timeDelta) = 0;
  virtual ~Position() = default;
  friend struct std::formatter<Position>;
};

class StaticPosition : public Position {
public:
  StaticPosition(const raylib::Vector2 &position) : _position(position) {}
  raylib::Vector2 operator()() const override { return _position; }
  void update([[maybe_unused]] float timeDelta) override {}

private:
  const raylib::Vector2 _position;
  friend struct std::formatter<StaticPosition>;
};

class RotatingPosition : public Position {
public:
  RotatingPosition(const raylib::Vector2 &position, float radius,
                   float velocity)
      : _position(position), _offset(raylib::Vector2{radius, 0}),
        _velocity(velocity) {}

  raylib::Vector2 operator()() const override { return _position + _offset; }

  void update(float timeDelta) override {
    _offset = _offset.Rotate(_velocity * timeDelta);
  }

private:
  const raylib::Vector2 _position;
  raylib::Vector2 _offset;
  float _velocity;

  friend struct std::formatter<RotatingPosition>;
};

template <> struct std::formatter<Position> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const Position &p, FormatContext &ctx) const {
    if (dynamic_cast<const StaticPosition *>(&p)) {
      return std::format_to(ctx.out(), "{}",
                            dynamic_cast<const StaticPosition &>(p));
    } else if (dynamic_cast<const RotatingPosition *>(&p)) {
      return std::format_to(ctx.out(), "{}",
                            dynamic_cast<const RotatingPosition &>(p));
    } else {
      return std::format_to(ctx.out(), "{}", p());
    }
  }
};

template <> struct std::formatter<StaticPosition> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const StaticPosition &p, FormatContext &ctx) const {
    return std::format_to(ctx.out(), "StaticPosition(position: {})",
                          p._position);
  }
};

template <> struct std::formatter<RotatingPosition> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const RotatingPosition &p, FormatContext &ctx) const {
    return std::format_to(ctx.out(),
                          "RotatingPosition(center: {}, offset: {}, omega: {})",
                          p._position, p._offset, p._velocity);
  }
};
