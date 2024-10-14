#include "Charge.hpp"
#include "FieldLine.hpp"
#include "Grid.hpp"
#include "Probe.hpp"
#include "defs.hpp"
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <ostream>
#include <print>
#include <ranges>
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
          Charge{pos, std::make_unique<charge::ConstantStrength>(strength)});
    } else if (strength.is_string()) {
      const std::string func = strength.get<std::string>();
      charges.push_back(
          Charge{pos, std::make_unique<charge::VariableStrength>(func)});
    }
  }
}

int main(int argc, char const *argv[]) {
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

      std::cerr
          << std::format(
                 "Invalid size spec: '{}'. Expected format is 'WIDTHxHEIGHT'",
                 size_spec)
          << std::endl;
    }
  }

  std::vector<Charge> charges{};
  {
    auto scenarion_result = load_scenario_json(scenario);

    if (!scenarion_result) {
      std::cerr << "Failed to load scenario: " << scenario << std::endl;
      return 1;
    }

    auto data = scenarion_result.value();

    std::cout << "Loaded scenario: " << scenario << std::endl;

    load_charges_from_json(charges, data);
  };

  std::cout << std::format("Loaded {} charge(s)", charges.size()) << std::endl;
  std::cout << "Charges: [" << std::endl;
  for (const Charge &charge : charges) {
    std::cout << std::format("\t{}", charge) << std::endl;
  }
  std::cout << "]" << std::endl;

  Probe probe(std::make_unique<position::Rotating>(raylib::Vector2{0, 0}, 1.f,
                                                   PI / 6.f),
              raylib::Color::Green());
  probe.scale = 1.5f;

  auto last_screen_size = raylib::Vector2(SCREEN_WIDTH, SCREEN_HEIGHT);

  Grid grid(last_screen_size, grid_spacing, raylib::Color::LightGray(),
            raylib::Color::Gray());
  grid.resize(last_screen_size, grid_spacing);

  double time = 0.0f;
  raylib::Color textColor(raylib::Color::DarkGray());
  raylib::Window w(SCREEN_WIDTH, SCREEN_HEIGHT,
                   "ELEKTROVIZ - A simple simulation of electric fields",
                   FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);

  w.SetTargetFPS(60);

  float zoom = 200.f / GLOBAL_SCALE;
  raylib::Camera2D camera({0, 0}, {0, 0}, 0.0f, zoom);

  std::vector<FieldLine> field_lines;

  for (const auto &charge : charges) {
    for (auto angle = 0.0f; angle < 2 * PI; angle += 2 * PI / 16.f) {
      field_lines.push_back(FieldLine{
          charge.position() +
              raylib::Vector2{cos(angle), sin(angle)}.Scale(0.2f),
          raylib::Vector2{cos(angle), sin(angle)}, raylib::Color::Red()});
    }
  }

  // Main game loop
  while (!w.ShouldClose()) // Detect window close button or ESC key
  {
    time = w.GetTime();
    auto frameTime = w.GetFrameTime();

    if (auto screen_size = w.GetSize(); !screen_size.Equals(last_screen_size)) {
      last_screen_size = screen_size;
      // make 0,0 the center of the screen
      camera.SetOffset(screen_size / 2.f);
      camera.SetZoom(zoom);
      grid.resize(screen_size / zoom, grid_spacing / zoom);
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
    for (auto &field_line : field_lines) {
      field_line.draw();
    }
    for (auto &charge : charges) {
      charge.draw();
    }
    probe.draw();

    camera.EndMode();

    auto fps_text = std::format("FPS: {}", GetFPS());
    auto text_pos = raylib::Vector2{10, 10};
    DrawText(fps_text, text_pos.x, text_pos.y, FONT_SIZE, textColor);

    // window size
    auto window_text =
        std::format("Window size: {}x{}", w.GetWidth(), w.GetHeight());
    DrawText(window_text, text_pos.x, text_pos.y + FONT_SIZE, FONT_SIZE,
             textColor);

    auto scenario_text = std::format("Scenario: {}", scenario);
    DrawText(scenario_text, text_pos.x, text_pos.y + 2 * FONT_SIZE, FONT_SIZE,
             textColor);

    w.EndDrawing();
  }

  return 0;
}
