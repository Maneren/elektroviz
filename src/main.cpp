#include "Charge.hpp"
#include "FieldLine.hpp"
#include "Grid.hpp"
#include "HeatMap.hpp"
#include "Plot.hpp"
#include "Position.hpp"
#include "Probe.hpp"
#include "defs.hpp"
#include "field.hpp"
#include "raylib.h"
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

std::optional<nlohmann::json> load_scenario_json(const std::string &scenario) {
  auto scenarioFile = std::ifstream{"scenarios/" + scenario};
  return scenarioFile ? std::optional{nlohmann::json::parse(scenarioFile)}
                      : std::nullopt;
}

std::vector<Charge> load_charges_from_json(nlohmann::json data) {
  std::vector<Charge> charges{};
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
  return charges;
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
    for (int i = 2; i < argc; i++) {
      auto size_spec = std::string{argv[i]};
      if (size_spec.starts_with("-g")) {
        size_spec = size_spec.substr(2);
        grid_spacing = raylib::Vector2{
            std::stof(size_spec.substr(0, size_spec.find("x"))),
            std::stof(size_spec.substr(size_spec.find("x") + 1))
        };
        break;
      } else {
        std::println(std::cerr, "WARNING: Unknown argument: '{}'", size_spec);
      }
    }
  }

  auto scenarion_result = load_scenario_json(scenario);

  if (!scenarion_result.has_value()) {
    std::cerr << "Failed to load scenario: " << scenario << std::endl;
    return 1;
  }

  auto data = scenarion_result.value();

  std::cout << "Loaded scenario: " << scenario << std::endl;

  auto charges = load_charges_from_json(data);

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

  w.SetMinSize(800, 600);
  // w.Maximize();

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
  auto simulation_time = 0.0;

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
    simulation_time += frameTime;

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
      charge.update(frameTime, simulation_time);
    }
    grid.update(frameTime, simulation_time, charges);
    probe.update(frameTime, simulation_time, charges);
    if (!user_probes.empty()) {
      for (auto &user_probe : user_probes) {
        if (user_probe.has_value()) {
          user_probe->update(frameTime, simulation_time, charges);
        }
      }

      plot.update(frameTime, w.GetTime(), simulation_speed, user_probes);
    }

    field_lines.update(charges, camera.GetScreenToWorld(wanted_target));

    auto reverse_camera_matrix = raylib::Matrix(camera.GetMatrix()).Invert();

    background.update([&reverse_camera_matrix, &charges](size_t x, size_t y) {
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
    for (const auto &charge : charges) {
      charge.draw();
    }
    probe.draw();
    for (auto &user_probe : user_probes) {
      if (user_probe.has_value()) {
        user_probe->draw();
      }
    }

    camera.EndMode();

    {
      auto gradient_height = 60.f;
      raylib::Rectangle{
          screen_size.x * 0.5f,
          screen_size.y - gradient_height - 5.f,
          screen_size.x * 0.2f,
          gradient_height
      }
          .DrawGradientH(Charge::NEGATIVE, raylib::Color::Black());

      raylib::Rectangle{
          screen_size.x * 0.7f,
          screen_size.y - gradient_height - 5.f,
          screen_size.x * 0.2f,
          gradient_height
      }
          .DrawGradientH(raylib::Color::Black(), Charge::POSITIVE);

      raylib::DrawText(
          std::format("-1E10"),
          screen_size.x * 0.5f + 5.f,
          screen_size.y - gradient_height,
          FONT_SIZE_SMALL,
          raylib::Color::RayWhite()
      );

      raylib::DrawText(
          "0",
          screen_size.x * 0.7f,
          screen_size.y - gradient_height,
          FONT_SIZE_SMALL,
          raylib::Color::RayWhite()
      );

      auto mid_text = std::format("Electric potential [V]");
      raylib::DrawText(
          mid_text,
          screen_size.x * 0.7f -
              raylib::MeasureText(mid_text, FONT_SIZE_SMALL) / 2.f,
          screen_size.y - 7.f - FONT_SIZE_SMALL,
          FONT_SIZE_SMALL,
          raylib::Color::RayWhite()
      );

      auto max_text = std::format("1E10");
      raylib::DrawText(
          max_text,
          screen_size.x * 0.9f -
              raylib::MeasureText(max_text, FONT_SIZE_SMALL) - 5.f,
          screen_size.y - gradient_height,
          FONT_SIZE_SMALL,
          raylib::Color::RayWhite()
      );

      raylib::Rectangle{
          screen_size.x * 0.5f,
          screen_size.y - gradient_height - 5.f,
          screen_size.x * 0.4f,
          gradient_height
      }
          .DrawLines(raylib::Color::White(), 4.f);
    }

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
        std::format("Speed: {:.4g}x", simulation_speed),
        text_pos_x,
        bottom_edge - FONT_SIZE - 5,
        FONT_SIZE,
        textColor
    );

    auto mouse_in_world = get_mouse_in_world(camera);
    auto shift_down = raylib::Keyboard::IsKeyDown(KEY_LEFT_SHIFT) ||
                      raylib::Keyboard::IsKeyDown(KEY_RIGHT_SHIFT);

    {
      auto scroll = raylib::Mouse::GetWheelMove();
      if (std::abs(scroll) > 0) {
        auto modified_charge = false;
        if (shift_down) {
          for (auto &charge : charges | views::reverse) {
            if (charge.contains(mouse_in_world)) {
              charge.modifier(1.f + scroll * 0.05f);
              modified_charge = true;
              break;
            }
          }
        } else if (!modified_charge) {
          zoom_modifier = std::clamp(zoom_modifier + scroll * 0.05f, 0.5f, 3.f);
          recalculate_zoom();
          resize_grid();
        }
      }
    }

    if (raylib::Mouse::IsButtonDown(MOUSE_BUTTON_MIDDLE) && !button_active) {
      if (!selected_charge_idx) {
        auto reversed_charges = charges | views::reverse;
        auto selected_charge = ranges::find_if(
            reversed_charges,
            [&mouse_in_world](const auto &charge) {
              return charge.contains(mouse_in_world);
            }
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
      if (shift_down) {
        for (auto &probe : user_probes) {
          if (!probe.has_value())
            continue;

          if (probe->contains(mouse_in_world)) {
            probe = std::nullopt;
            break;
          }
        }
        if (ranges::all_of(user_probes, [](auto &p) {
              return !p.has_value();
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

        user_probes.emplace_back(Probe{
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
