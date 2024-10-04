#include "defs.hpp"
#include <raylib-cpp.hpp>

class Position {
public:
  virtual raylib::Vector2 operator()() const = 0;
  virtual void update(float timeDelta) = 0;
  virtual ~Position() = default;
};

class StaticPosition : public Position {
public:
  StaticPosition(const raylib::Vector2 &position) : _position(position) {}
  raylib::Vector2 operator()() const override { return _position; }
  void update([[maybe_unused]] float timeDelta) override {}

private:
  const raylib::Vector2 _position;
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
};
