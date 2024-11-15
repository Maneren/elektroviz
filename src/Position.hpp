#pragma once
#include "BoundingRectangle.hpp"
#include <Vector2.hpp>
#include <format>

class Position {
public:
  virtual raylib::Vector2 operator()() const = 0;
  virtual void update(const float timeDelta, const float elapsedTime) = 0;
  virtual BoundingRectangle bounding_rectangle() const = 0;
  virtual ~Position() = default;
  friend struct std::formatter<Position>;
};

namespace position {

class Static : public Position {
public:
  explicit Static(const raylib::Vector2 &position) : _position(position) {}
  raylib::Vector2 operator()() const override { return _position; }
  void update(
      [[maybe_unused]] const float timeDelta,
      [[maybe_unused]] const float elapsedTime
  ) override {
    // static position doesn't need update, provided just for interface sake
  }
  BoundingRectangle bounding_rectangle() const override {
    return {_position, {0.f, 0.f}};
  }

private:
  const raylib::Vector2 _position;
  friend struct std::formatter<Static>;
};

class Rotating : public Position {
public:
  explicit Rotating(
      const raylib::Vector2 &position, const float radius, const float velocity
  )
      : _position(position), _radius(radius),
        _offset(raylib::Vector2{radius, 0}), _velocity(velocity) {}
  raylib::Vector2 operator()() const override { return _position + _offset; }
  void update([[maybe_unused]] const float timeDelta, const float elapsedTime)
      override {
    _offset = raylib::Vector2{_radius, 0}.Rotate(_velocity * elapsedTime);
  }
  BoundingRectangle bounding_rectangle() const override {
    return {
        _position - raylib::Vector2{_radius, _radius},
        raylib::Vector2{_radius, _radius} * 2.f,
    };
  }

private:
  const raylib::Vector2 _position;
  const float _radius;
  raylib::Vector2 _offset;
  float _velocity;

  friend struct std::formatter<Rotating>;
};

} // namespace position

template <> struct std::formatter<Position> {
  constexpr auto parse(std::format_parse_context const &ctx) const {
    return ctx.begin();
  }
  template <typename FormatContext>
  auto format(const Position &p, FormatContext &ctx) const {
    if (dynamic_cast<const position::Static *>(&p)) {
      return std::format_to(
          ctx.out(), "{}", dynamic_cast<const position::Static &>(p)
      );
    } else if (dynamic_cast<const position::Rotating *>(&p)) {
      return std::format_to(
          ctx.out(), "{}", dynamic_cast<const position::Rotating &>(p)
      );
    } else {
      return std::format_to(ctx.out(), "{}", p());
    }
  }
};

template <> struct std::formatter<position::Static> {
  constexpr auto parse(std::format_parse_context const &ctx) const {
    return ctx.begin();
  }
  template <typename FormatContext>
  auto format(const position::Static &p, FormatContext &ctx) const {
    return std::format_to(
        ctx.out(), "StaticPosition(position: {})", p._position
    );
  }
};

template <> struct std::formatter<position::Rotating> {
  constexpr auto parse(std::format_parse_context const &ctx) const {
    return ctx.begin();
  }
  template <typename FormatContext>
  auto format(const position::Rotating &p, FormatContext &ctx) const {
    return std::format_to(
        ctx.out(),
        "RotatingPosition(center: {}, offset: {}, omega: {})",
        p._position,
        p._offset,
        p._velocity
    );
  }
};
