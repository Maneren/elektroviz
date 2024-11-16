#include "Probe.hpp"
#include <Vector2.hpp>
#include <deque>
#include <vector>

class Plot {
public:
  Plot(
      const raylib::Vector2 position,
      const raylib::Vector2 size,
      const raylib::Color background_color = raylib::Color::LightGray()
  )
      : position(position), size(size), background_color(background_color),
        data({}) {};

  void draw() const;
  void
  update(const float timeDelta, const double elapsedTime, const Probe &probe);
  // void add_probe(std::shared_ptr<Probe> probe) { probes.push_back(probe); }
  void resize(const raylib::Vector2 position, const raylib::Vector2 size) {
    this->position = position;
    this->size = size;
  }

private:
  const float resolution = 30.f * 60.f; // 30 seconds at 60 samples per second
  raylib::Vector2 position;
  raylib::Vector2 size;
  raylib::Color background_color;
  double last_update = 0.0;

  std::vector<std::deque<float>> data;
  std::vector<std::shared_ptr<Probe>> probes;
};
