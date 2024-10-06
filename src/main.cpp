#include "Camera2D.hpp"
#include "Charge.hpp"
#include "Grid.hpp"
#include "Probe.hpp"
#include "Vector2.hpp"
#include "defs.hpp"
#include "raylib.h"
#include <format>
#include <fstream>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <print>
#include <raylib-cpp.hpp>
#include <string>
#include <vector>

int main(int argc, char const *argv[]) {
  int flags = FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE;

  raylib::Color textColor(GRAY);
  raylib::Window w(SCREEN_WIDTH, SCREEN_HEIGHT,
                   "ELEKTROVIZ - A simple simulation of electric fields",
                   flags);

  w.SetTargetFPS(60);

  raylib::Camera2D camera({0, 0}, {0, 0}, 0.0f, 1.0f);

  std::string scenario = "0.json";
  if (argc > 1) {
    scenario = std::string{argv[1]} + ".json";
  }

  auto scenarioFile = std::ifstream{"scenarios/" + scenario};
  if (!scenarioFile) {
    std::println("Failed to open scenario file: {}", scenario);
    return 1;
  }

  json data = json::parse(scenarioFile);

  std::println("Loaded scenario: {}", scenario);

  std::vector<Charge> charges{};

  for (const auto &charge : data["charges"]) {
    auto position = charge["position"];
    raylib::Vector2 pos{position["x"], position["y"]};

    auto strength = charge["strength"];
    if (strength.is_number()) {
      charges.push_back(
          Charge{pos, std::make_unique<ConstantChargeStrength>(strength)});
    } else if (strength.is_string()) {
      const std::string func = strength.get<std::string>();
      charges.push_back(
          Charge{pos, std::make_unique<VariableChargeStrength>(func)});
    }
  }

  std::println("Loaded {} charge(s)", charges.size());
  std::println("Charges: [");
  for (const auto &charge : charges) {
    std::println("\t{}", charge);
  }
  std::println("]");

  Probe probe(
      std::make_unique<RotatingPosition>(raylib::Vector2{0, 0}, 1.f, PI / 6.f),
      GREEN);

  raylib::Vector2 last_screen_size = w.GetSize();

  raylib::Vector2 grid_spacing = {50.f, 30.f};
  Grid grid(last_screen_size, grid_spacing, LIGHTGRAY);

  // Main game loop
  while (!w.ShouldClose()) // Detect window close button or ESC key
  {
    auto time = w.GetTime();
    auto frameTime = w.GetFrameTime();

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
