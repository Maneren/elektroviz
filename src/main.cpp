#include "Charge.hpp"
#include "Probe.hpp"
#include "defs.hpp"
#include "field.hpp"
#include "raylib.h"
#include <format>
#include <print>
#include <raylib-cpp.hpp>
#include <vector>

int main() {
  SetConfigFlags(FLAG_MSAA_4X_HINT);

  raylib::Color textColor(LIGHTGRAY);
  raylib::Window w(SCREEN_WIDTH, SCREEN_HEIGHT,
                   "ELEKTROVIZ - A simple simulation of electric fields");

  SetTargetFPS(60);

  auto screen_center = raylib::Vector2{static_cast<float>(SCREEN_WIDTH / 2.),
                                       static_cast<float>(SCREEN_HEIGHT / 2.)};

  // make 0,0 the center of the screen
  Camera2D camera(screen_center, {0, 0}, 0.0f, 1.0f);

  Charge charge({0, 0}, ConstantChargeStrength(1.0f));
  charge.update(0.0f);

  std::vector<Charge> charges = {charge};

  raylib::Vector2 point(1, 0);
  auto field = eFieldAt(point, charges);
  std::println("Charge at ({}, {}) is {} V/m", point.x, point.y,
               field.Length());

  Probe probe({0, 1}, GREEN);

  // Main game loop
  while (!w.ShouldClose()) // Detect window close button or ESC key
  {
    auto frameTime = GetFrameTime();
    // Update

    for (auto &charge : charges) {
      charge.update(frameTime);
    }
    probe.update(charges);

    // Draw
    BeginDrawing();
    ClearBackground(RAYWHITE);
    BeginMode2D(camera);

    for (auto &charge : charges) {
      charge.draw();
    }
    probe.draw<10.f, true>();

    EndMode2D();
    EndDrawing();
  }

  return 0;
}
