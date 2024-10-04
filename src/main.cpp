#include "Camera2D.hpp"
#include "Charge.hpp"
#include "Probe.hpp"
#include "defs.hpp"
#include "raylib.h"
#include <memory>
#include <print>
#include <raylib-cpp.hpp>
#include <vector>

int main() {
  SetConfigFlags(FLAG_MSAA_4X_HINT);

  raylib::Color textColor(LIGHTGRAY);
  raylib::Window w(SCREEN_WIDTH, SCREEN_HEIGHT,
                   "ELEKTROVIZ - A simple simulation of electric fields");

  w.SetTargetFPS(60);

  auto screen_center = raylib::Vector2{static_cast<float>(SCREEN_WIDTH / 2.),
                                       static_cast<float>(SCREEN_HEIGHT / 2.)};

  // make 0,0 the center of the screen
  raylib::Camera2D camera(screen_center, {0, 0}, 0.0f, 1.0f);

  std::vector<Charge> charges = {
      {{-1, -1}, ConstantChargeStrength(1.f)},
      {{1, -1}, ConstantChargeStrength(2.f)},
      {{1, 1}, ConstantChargeStrength(-3.f)},
      {{-1, 1}, ConstantChargeStrength(-4.f)},
  };

  Probe probe(
      std::make_unique<RotatingPosition>(raylib::Vector2{0, 0}, 1.f, PI / 6.f),
      GREEN);

  // Main game loop
  while (!w.ShouldClose()) // Detect window close button or ESC key
  {
    auto time = GetTime();
    auto frameTime = GetFrameTime();

    // Update
    for (auto &charge : charges) {
      charge.update(time);
    }
    probe.update(frameTime, charges);

    // Draw
    w.BeginDrawing();
    w.ClearBackground(RAYWHITE);
    camera.BeginMode();

    for (auto &charge : charges) {
      charge.draw();
    }
    probe.draw<10.f, true>();

    camera.EndMode();
    w.EndDrawing();
  }

  return 0;
}
