#include "Camera2D.hpp"
#include "Charge.hpp"
#include "Grid.hpp"
#include "Probe.hpp"
#include "Vector2.hpp"
#include "defs.hpp"
#include "raylib.h"
#include <format>
#include <memory>
#include <print>
#include <raylib-cpp.hpp>
#include <vector>

int main() {
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);

  raylib::Color textColor(GRAY);
  raylib::Window w(SCREEN_WIDTH, SCREEN_HEIGHT,
                   "ELEKTROVIZ - A simple simulation of electric fields");

  w.SetTargetFPS(60);

  raylib::Camera2D camera({0, 0}, {0, 0}, 0.0f, 1.0f);

  std::vector<Charge> charges = {
      {{-1, -1}, ConstantChargeStrength(1.f)},
      {{1, -1}, ConstantChargeStrength(2.f)},
      {{1, 1}, ConstantChargeStrength(-3.f)},
      {{-1, 1}, ConstantChargeStrength(-4.f)},
  };

  Probe probe(
      std::make_unique<RotatingPosition>(raylib::Vector2{0, 0}, 1.f, PI / 6.f),
      GREEN);

  raylib::Vector2 last_screen_size = w.GetSize();

  raylib::Vector2 grid_spacing = {50.f, 30.f};
  Grid grid(last_screen_size, grid_spacing, LIGHTGRAY);

  // Main game loop
  while (!w.ShouldClose()) // Detect window close button or ESC key
  {
    auto time = GetTime();
    auto frameTime = GetFrameTime();

    if (auto screen_size = w.GetSize(); !screen_size.Equals(last_screen_size)) {
      // make 0,0 the center of the screen
      camera.SetOffset(w.GetSize() / 2.f);
      grid.resize(screen_size, grid_spacing);
    }

    // Update
    for (auto &charge : charges) {
      charge.update(time);
    }
    grid.update(frameTime, charges);
    probe.update(frameTime, charges);

    // Draw
    w.BeginDrawing();
    w.ClearBackground(RAYWHITE);
    camera.BeginMode();

    grid.draw();
    for (auto &charge : charges) {
      charge.draw();
    }
    probe.draw<10.f, true>();

    auto fps_text = std::format("FPS: {}", GetFPS());
    auto text_pos = (-w.GetSize() / 2.f) + raylib::Vector2{10, 10};
    DrawText(fps_text, text_pos.x, text_pos.y, FONT_SIZE, textColor);

    // window size
    auto window_text =
        std::format("Window size: {}x{}", w.GetWidth(), w.GetHeight());
    text_pos = (-w.GetSize() / 2.f) + raylib::Vector2{10, 40};
    DrawText(window_text, text_pos.x, text_pos.y, FONT_SIZE, textColor);

    camera.EndMode();
    w.EndDrawing();
  }

  return 0;
}
