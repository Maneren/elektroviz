#include "BoundingSquare.hpp"
#include "Charge.hpp"
#include "FieldLine.hpp"
#include "Grid.hpp"
#include "Position.hpp"
#include "Probe.hpp"
#include "defs.hpp"
#include "field.hpp"
#include "parallel.hpp"
#include "utils.hpp"
#include <Camera2D.hpp>
#include <Color.hpp>
#include <Functions.hpp>
#include <Image.hpp>
#include <Texture.hpp>
#include <Vector2.hpp>
#include <Window.hpp>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <raylib.h>
#include <span>
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
      charges.emplace_back(
          pos, std::make_unique<charge::ConstantStrength>(strength));
    } else if (strength.is_string()) {
      const std::string func = strength.get<std::string>();
      charges.emplace_back(pos,
                           std::make_unique<charge::VariableStrength>(func));
    }
  }
}

BoundingSquare world_bounding_square(const std::span<Charge> &charges,
                                     const Probe &probe) {
  BoundingSquare bounding_square = {{0.f, 0.f}, {0.f, 0.f}};

  for (const auto &charge : charges) {
    bounding_square += charge.bounding_square();
  }

  bounding_square += probe.bounding_square();

  return bounding_square;
}

int main(int argc, char const *argv[]) {
  std::string scenario = "0.json";
  if (argc > 1) {
    scenario = std::string{argv[1]} + ".json";
  }

  raylib::Vector2 grid_spacing = {50.f, 50.f};
  if (argc > 2) {
    auto size_spec = std::string{argv[2]};
    if (size_spec.contains("x")) {
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
  }

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
  auto half_screen_size = last_screen_size / 2.f;
  auto world_size = last_screen_size / GLOBAL_SCALE;

  Grid grid(last_screen_size, grid_spacing, raylib::Color::RayWhite(),
            {150, 150, 150, 255});
  grid.resize(last_screen_size, grid_spacing);

  double time = 0.0f;
  raylib::Color textColor(raylib::Color::LightGray());
  raylib::Window w(SCREEN_WIDTH, SCREEN_HEIGHT,
                   "ELEKTROVIZ - A simple simulation of electric fields",
                   FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);

  // w.SetTargetFPS(60);

  auto zoom = 2.f;
  raylib::Camera2D camera({0, 0}, {0, 0}, 0.f, zoom);

  FieldLines field_lines{LINES_PER_CHARGE};

  constexpr int BACKGROUND_SUBSAMPLING = 2;

  raylib::Image background_image = raylib::Image::Color(
      SCREEN_WIDTH / BACKGROUND_SUBSAMPLING,
      SCREEN_HEIGHT / BACKGROUND_SUBSAMPLING, raylib::Color::Blank());
  auto background_texture = raylib::Texture2D(background_image);

  // Main game loop
  while (!w.ShouldClose()) // Detect window close button or ESC key
  {
    time = w.GetTime();
    auto frameTime = w.GetFrameTime();

    if (auto screen_size = w.GetSize(); !screen_size.Equals(last_screen_size)) {
      last_screen_size = screen_size;
      half_screen_size = screen_size / 2.f;
      world_size = screen_size / GLOBAL_SCALE;

      auto bounding_square = world_bounding_square(charges, probe);
      auto bounding_size =
          bounding_square.size * GLOBAL_SCALE + raylib::Vector2{150.f, 150.f};
      zoom = std::min(last_screen_size.x / bounding_size.x,
                      last_screen_size.y / bounding_size.y);

      // Make 0,0 the center of the screen
      camera.SetOffset(half_screen_size);
      camera.SetZoom(zoom);
      grid.resize(screen_size / zoom, grid_spacing / zoom);
      background_texture.Unload();
      background_image.Resize(screen_size.x / BACKGROUND_SUBSAMPLING,
                              screen_size.y / BACKGROUND_SUBSAMPLING);
      background_texture = raylib::Texture2D(background_image);
    }

    // Update
    for (auto &charge : charges) {
      charge.update(frameTime, time);
    }
    // grid.update(frameTime, time, charges);
    probe.update(frameTime, time, charges);
    // field_lines.update(charges);

    // SAFETY: each thread will access independent portion of the image and the
    // image is internally composed of Colors, so it's safe to treat it as such
    // parallel::for_each<raylib::Color>(
    //     std::span(static_cast<raylib::Color *>(background_image.data),
    //               background_image.width * background_image.height),
    //     [&background_image, &charges,
    //      half_world_size = world_size / 2.f](auto i, auto &pixel) {
    //       auto x = i % background_image.width;
    //       auto y = i / background_image.width;
    //
    //       auto x_screen = static_cast<float>(BACKGROUND_SUBSAMPLING * x);
    //       auto y_screen = static_cast<float>(BACKGROUND_SUBSAMPLING * y);
    //       auto position = raylib::Vector2{x_screen, y_screen} / GLOBAL_SCALE
    //       -
    //                       half_world_size;
    //
    //       auto potencial = field::potential(position, charges) / 2e10f;
    //
    //       auto color = lerpColor3(Charge::NEGATIVE, raylib::Color::Black(),
    //                               Charge::POSITIVE, sigmoid(potencial));
    //
    //       pixel = color;
    //     });
    //
    // background_texture.Update(background_image.data);

    // Draw
    w.BeginDrawing();

    camera.BeginMode();

    w.ClearBackground(raylib::Color::Black());
    // background_texture.Draw(-half_screen_size, 0.f,
    //                         static_cast<float>(BACKGROUND_SUBSAMPLING));

    // grid.draw();
    // field_lines.draw();
    for (auto &charge : charges) {
      charge.draw();
    }
    probe.draw();

    camera.EndMode();

    auto text_pos_x = 10;
    auto text_pos_y = 10;

    auto fps_text = std::format("FPS: {}", w.GetFPS());
    raylib::DrawText(fps_text, text_pos_x, text_pos_y, FONT_SIZE, textColor);

    auto window_text =
        std::format("Window size: {}x{}", w.GetWidth(), w.GetHeight());
    raylib::DrawText(window_text, text_pos_x, text_pos_y + FONT_SIZE, FONT_SIZE,
                     textColor);

    auto scenario_text = std::format("Scenario: {}", scenario);
    raylib::DrawText(scenario_text, text_pos_x, text_pos_y + 2 * FONT_SIZE,
                     FONT_SIZE, textColor);

    w.EndDrawing();
  }

  return 0;
}
