#include "Probe.hpp"
#include <Color.hpp>
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

  void draw(const std::vector<raylib::Color> &probe_colors) const;
  void update(
      const float timeDelta,
      const double elapsedTime,
      const std::vector<std::optional<Probe>> &probe
  );
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
