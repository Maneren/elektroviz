#include "Charge.hpp"
#include "FieldLine.hpp"
#include "Grid.hpp"
#include "HeatMap.hpp"
#include "Plot.hpp"
#include "Position.hpp"
#include "Probe.hpp"
#include "defs.hpp"
#include "field.hpp"
#include "raymath.h"
#include "utils.hpp"
#include <Camera2D.hpp>
#include <Color.hpp>
#include <Functions.hpp>
#include <Image.hpp>
#include <Keyboard.hpp>
#include <Matrix.hpp>
#include <Mouse.hpp>
#include <Rectangle.hpp>
#include <Texture.hpp>
#include <Vector2.hpp>
#include <Window.hpp>
#include <algorithm>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <ostream>
#include <random>
#include <ranges>
extern "C" {
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>
}
#include <span>
#include <string>
#include <vector>

namespace ranges = std::ranges;
namespace views = std::views;
namespace placeholders = std::placeholders;

static std::random_device dev;
static std::mt19937 rng(dev());
static std::uniform_int_distribution<unsigned char> dist256(10, 255);

raylib::Color generate_random_color() {
  unsigned char red;
  unsigned char green;
  unsigned char blue;

  do {
    red = dist256(rng);
    green = dist256(rng);
    blue = dist256(rng);
  } while (red + green + blue < 256);

  return raylib::Color{red, green, blue};
}

std::optional<nlohmann::json> load_scenario_json(std::string scenario) {
  auto scenarioFile = std::ifstream{"scenarios/" + scenario};
  return scenarioFile ? std::optional{nlohmann::json::parse(scenarioFile)}
                      : std::nullopt;
}

void load_charges_from_json(std::vector<Charge> &charges, nlohmann::json data) {
  for (const auto &charge : data["charges"]) {
    auto position = charge["position"];
    raylib::Vector2 pos{position["x"], position["y"]};
    pos.y *= -1.f;
    pos *= 100.f;

    auto strength = charge["strength"];
    if (strength.is_number()) {
      charges.emplace_back(
          pos, std::make_unique<charge::ConstantStrength>(strength)
      );
    } else if (strength.is_string()) {
      const std::string func = strength.get<std::string>();
      charges.emplace_back(
          pos, std::make_unique<charge::VariableStrength>(func)
      );
    }
  }
}

raylib::Vector2 get_mouse_in_world(raylib::Camera2D camera) {
  return camera.GetScreenToWorld(raylib::Mouse::GetPosition());
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
      grid_spacing = raylib::Vector2{
          std::stof(size_spec.substr(0, size_spec.find("x"))),
          std::stof(size_spec.substr(size_spec.find("x") + 1))
      };
    } else {
      std::cerr << std::format(
                       "Invalid size spec: '{}'. Expected format "
                       "is 'WIDTHxHEIGHT'",
                       size_spec
                   )
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

  Probe probe(
      std::make_unique<position::Rotating>(
          raylib::Vector2{0, 0}, 100.f, PI / 6.f
      ),
      raylib::Color::Green()
  );

  std::vector<raylib::Color> user_probe_colors{
      raylib::Color::Yellow(),
      raylib::Color::Magenta(),
      raylib::Color::SkyBlue(),
      raylib::Color::Red(),
      raylib::Color::Purple(),
  };
  std::vector<std::optional<Probe>> user_probes{};

  auto screen_size = raylib::Vector2(SCREEN_WIDTH, SCREEN_HEIGHT);
  auto half_screen_size = screen_size / 2.f;

  raylib::Color textColor(raylib::Color::LightGray());
  raylib::Window w(
      SCREEN_WIDTH,
      SCREEN_HEIGHT,
      "ELEKTROVIZ - A simple visualizer of an electric field",
      FLAG_WINDOW_RESIZABLE
  );

  w.SetTargetFPS(60);

  auto zoom_modifier = 1.f;
  raylib::Camera2D camera(half_screen_size, {0.f, 0.f}, 0.f, 1.f);
  raylib::Vector2 wanted_target{0.f, 0.f};

  Grid grid{
      screen_size,
      grid_spacing,
      camera,
      raylib::Color::RayWhite(),
      {150, 150, 150, 255}
  };

  FieldLines field_lines{LINES_PER_CHARGE};

  HeatMap background{
      raylib::Vector2{0, 0},
      screen_size / BACKGROUND_SUBSAMPLING,
      Charge::POSITIVE,
      raylib::Color::Black(),
      Charge::NEGATIVE,
      static_cast<float>(BACKGROUND_SUBSAMPLING)
  };

  auto simulation_speed = 1.f;
  auto time = 0.0;

  Plot plot(
      {screen_size.x * 0.3f, 0.0},
      {screen_size.x * 0.7f, screen_size.y / 4.f},
      raylib::Color::Black(),
      raylib::Color::RayWhite()
  );

  std::optional<int> selected_charge_idx = std::nullopt;

  auto resize_grid = [&] { grid.resize(screen_size, grid_spacing, camera); };

  auto resize_plot = [&] {
    plot.resize(
        {screen_size.x * 0.4f, 0.0}, {screen_size.x * 0.6f, screen_size.y / 4.f}
    );
  };

  auto recalculate_zoom = [&] {
    camera.zoom =
        std::min(screen_size.x / 600.f, screen_size.y / 600.f) * zoom_modifier;
  };

  // Main game loop
  while (!w.ShouldClose()) // Detect window close button or ESC key
  {
    auto frameTime = w.GetFrameTime() * simulation_speed;
    time += frameTime;

    if (!wanted_target.Equals(camera.target)) {
      camera.target = (wanted_target / 5.f + camera.target) / (1.f / 5.f + 1);
      resize_grid();
    }

    if (w.IsResized()) {
      screen_size = w.GetSize();
      half_screen_size = screen_size / 2.f;

      // Make 0,0 the center of the screen
      camera.SetOffset(half_screen_size);
      recalculate_zoom();

      resize_grid();
      resize_plot();

      background.resize(screen_size / BACKGROUND_SUBSAMPLING);
    }

    // Update
    for (auto &charge : charges) {
      charge.update(frameTime, time);
    }
    grid.update(frameTime, time, charges);
    probe.update(frameTime, time, charges);
    if (!user_probes.empty()) {
      for (auto &user_probe : user_probes) {
        if (user_probe.has_value()) {
          user_probe->update(frameTime, time, charges);
        }
      }

      plot.update(frameTime, time, user_probes);
    }

    field_lines.update(charges, camera.GetScreenToWorld(wanted_target));

    auto reverse_camera_matrix = raylib::Matrix(camera.GetMatrix()).Invert();

    background.update([&](int x, int y) {
      auto x_pos = static_cast<float>(x * BACKGROUND_SUBSAMPLING);
      auto y_pos = static_cast<float>(y * BACKGROUND_SUBSAMPLING);

      auto position =
          raylib::Vector2{x_pos, y_pos}.Transform(reverse_camera_matrix);

      return sigmoid(field::potential(position, charges));
    });

    // Draw
    w.BeginDrawing();

    background.draw();

    camera.BeginMode();

    grid.draw();
    field_lines.draw();
    for (auto &charge : charges) {
      charge.draw();
    }
    probe.draw();
    for (auto &user_probe : user_probes) {
      if (user_probe.has_value()) {
        user_probe->draw();
      }
    }

    camera.EndMode();

    if (!user_probes.empty())
      plot.draw(user_probe_colors);

    auto text_pos_x = 10;
    auto text_pos_y = 10;

    auto fps_text = std::format("FPS: {}", w.GetFPS());
    raylib::DrawText(fps_text, text_pos_x, text_pos_y, FONT_SIZE, textColor);

    auto window_text =
        std::format("Window size: {}x{}", w.GetWidth(), w.GetHeight());
    raylib::DrawText(
        window_text, text_pos_x, text_pos_y + FONT_SIZE, FONT_SIZE, textColor
    );

    auto scenario_text = std::format("Scenario: {}", scenario);
    raylib::DrawText(
        scenario_text,
        text_pos_x,
        text_pos_y + 2 * FONT_SIZE,
        FONT_SIZE,
        textColor
    );

    float bottom_edge = static_cast<float>(w.GetHeight() - 45);

    int slow_button_state = GuiButton(
        raylib::Rectangle{5, bottom_edge, 80, 40},
        GuiIconText(ICON_PLAYER_PREVIOUS, "Slow")
    );
    int fast_button_state = GuiButton(
        raylib::Rectangle{95, bottom_edge, 80, 40},
        GuiIconText(ICON_PLAYER_NEXT, "Fast")
    );
    int normal_speed_button_state = GuiButton(
        raylib::Rectangle{185, bottom_edge, 80, 40},
        GuiIconText(ICON_PLAYER_PLAY, "Normal")
    );

    if (slow_button_state && simulation_speed > .25f)
      simulation_speed /= 2;
    if (fast_button_state && simulation_speed < 4.f)
      simulation_speed *= 2;
    if (normal_speed_button_state)
      simulation_speed = 1.f;

    auto button_active =
        fast_button_state || slow_button_state || normal_speed_button_state;

    raylib::DrawText(
        std::format("Speed: {:.4g}", simulation_speed),
        text_pos_x,
        bottom_edge - FONT_SIZE - 5,
        FONT_SIZE,
        textColor
    );

    auto scroll = raylib::Mouse::GetWheelMove();

    if (std::abs(scroll) > 0) {
      zoom_modifier = std::clamp(zoom_modifier + scroll * 0.05f, 0.5f, 3.f);
      recalculate_zoom();
      resize_grid();
    }

    if (raylib::Mouse::IsButtonDown(MOUSE_BUTTON_MIDDLE) && !button_active) {
      auto mouse_in_world = get_mouse_in_world(camera);

      if (!selected_charge_idx) {
        auto reversed_charges = charges | views::reverse;
        auto selected_charge = ranges::find_if(
            reversed_charges,
            std::bind(&Charge::contains, placeholders::_1, mouse_in_world)
        );

        if (selected_charge != reversed_charges.end()) {
          selected_charge_idx =
              std::distance(charges.begin(), selected_charge.base() - 1);
        }
      } else {
        charges[*selected_charge_idx].position(mouse_in_world);
      }
    } else {
      selected_charge_idx = std::nullopt;
    }

    if (raylib::Mouse::IsButtonDown(MOUSE_BUTTON_RIGHT) && !button_active) {
      auto delta = raylib::Mouse::GetDelta().Scale(-1.0f / camera.zoom);
      camera.target =
          (Vector2ClampValue(Vector2Add(camera.target, delta), 0.f, 800.f));
      wanted_target = camera.target;
      resize_grid();
    }

    if (raylib::Mouse::IsButtonReleased(MOUSE_BUTTON_LEFT) && !button_active) {
      auto mouse_in_world = get_mouse_in_world(camera);

      if (raylib::Keyboard::IsKeyDown(KEY_LEFT_SHIFT) ||
          raylib::Keyboard::IsKeyDown(KEY_RIGHT_SHIFT)) {
        for (auto &probe : user_probes) {
          if (!probe.has_value())
            continue;

          if (probe->contains(mouse_in_world)) {
            probe = std::nullopt;
            break;
          }
        }
        if (ranges::all_of(user_probes, [](auto &probe) {
              return !probe.has_value();
            })) {
          user_probes.clear();
          plot.clear();
          wanted_target += raylib::Vector2{0.f, half_screen_size.y / 8.f};
          resize_grid();
        }
      } else {
        auto first_place = user_probes.empty();

        while (user_probe_colors.size() <= user_probes.size())
          user_probe_colors.push_back(generate_random_color());

        user_probes.push_back(Probe{
            std::make_unique<position::Static>(mouse_in_world),
            user_probe_colors[user_probes.size()],
            8.f,
            50.f
        });

        if (first_place) {
          wanted_target -= raylib::Vector2{0.f, half_screen_size.y / 8.f};
          resize_grid();
          resize_plot();
        }
      }
    }

    w.EndDrawing();
  }

  return 0;
}
