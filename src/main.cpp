#include "Charge.hpp"
#include "Grid.hpp"
#include "Probe.hpp"
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

std::optional<nlohmann::json> load_scenario_json(std::string scenario) {
  auto scenarioFile = std::ifstream{"scenarios/" + scenario};
  return scenarioFile ? std::optional{nlohmann::json::parse(scenarioFile)}
                      : std::nullopt;
}

void load_charges_from_json(std::vector<Charge> &charges, nlohmann::json data) {
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
}

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

  raylib::Vector2 grid_spacing = {50.f, 50.f};
  if (argc > 2) {
    std::string size_spec = std::string{argv[2]};
    if (size_spec.find("x") != std::string::npos) {
      grid_spacing =
          raylib::Vector2{std::stof(size_spec.substr(0, size_spec.find("x"))),
                          std::stof(size_spec.substr(size_spec.find("x") + 1))};
    } else {
      std::println(std::cerr,
                   "Invalid size spec: '{}'. Expected format is 'WIDTHxHEIGHT'",
                   size_spec);
    }
  }

  std::vector<Charge> charges{};
  {
    auto scenarion_result = load_scenario_json(scenario);

    if (!scenarion_result) {
      std::println(std::cerr, "Failed to load scenario: {}", scenario);
      return 1;
    }

    auto data = scenarion_result.value();

    std::println("Loaded scenario: {}", scenario);

    load_charges_from_json(charges, data);
  };

  std::println("Loaded {} charge(s)", charges.size());
  std::println("Charges: [");
  for (const Charge &charge : charges) {
    std::println("\t{}", charge);
  }
  std::println("]");

  Probe probe(
      std::make_unique<RotatingPosition>(raylib::Vector2{0, 0}, 1.f, PI / 6.f),
      GREEN);

  raylib::Vector2 last_screen_size = w.GetSize();

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
      charge.update(frameTime, time);
    }
    grid.update(frameTime, time, charges);
    probe.update(frameTime, time, charges);

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
    DrawText(window_text, text_pos.x, text_pos.y + FONT_SIZE, FONT_SIZE,
             textColor);

    auto scenario_text = std::format("Scenario: {}", scenario);
    DrawText(scenario_text, text_pos.x, text_pos.y + 2 * FONT_SIZE, FONT_SIZE,
             textColor);

    camera.EndMode();
    w.EndDrawing();
  }

  return 0;
}
