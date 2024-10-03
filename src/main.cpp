#include "Charge.hpp"
#include "field.hpp"
#include <format>
#include <print>
#include <raylib-cpp.hpp>
#include <vector>

int main() {
  raylib::Color textColor(LIGHTGRAY);
  raylib::Window w(SCREEN_WIDTH, SCREEN_HEIGHT,
                   "ELEKTROVIZ - A simple simulation of electric fields");

  SetTargetFPS(60);

  Charge charge({0, 0}, ConstantChargeStrength(10.0f));
  charge.update(0.0f);

  std::vector<Charge> charges = {charge};

  raylib::Vector2 point(1, 0);
  auto field = eFieldAt(point, charges);
  std::println("Charge at ({}, {}) is {} V/m", point.x, point.y,
               field.Length());

  // Main game loop
  while (!w.ShouldClose()) // Detect window close button or ESC key
  {
    auto frameTime = GetFrameTime();
    // Update

    for (auto &charge : charges) {
      charge.update(frameTime);
    }

    // Draw
    BeginDrawing();
    ClearBackground(RAYWHITE);
    textColor.DrawText("Congrats! You created your first window!", 190, 200,
                       20);
    charge.draw();
    EndDrawing();
  }

  return 0;
}
