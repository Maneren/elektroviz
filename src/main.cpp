#include "BoundingRectangle.hpp"
#include "Charge.hpp"
#include "FieldLine.hpp"
#include "Grid.hpp"
#include "Plot.hpp"
#include "Position.hpp"
#include "Probe.hpp"
#include "defs.hpp"
#include "field.hpp"
#include "parallel.hpp"
#include "raymath.h"
#include "utils.hpp"
#include <Camera2D.hpp>
#include <Color.hpp>
#include <Functions.hpp>
#include <Image.hpp>
#include <Keyboard.hpp>
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
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
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
namespace placeholders = std::placeholders;

static std::random_device dev;
static std::mt19937 rng(dev());
static std::uniform_int_distribution<unsigned char> dist256(0, 255);

raylib::Color generate_random_color() {
  unsigned char red = dist256(rng);
  unsigned char green = dist256(rng);
  unsigned char blue = dist256(rng);
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

BoundingRectangle
world_bounding_square(const std::span<Charge> &charges, const Probe &probe) {
  BoundingRectangle bounding_square = {{0.f, 0.f}, {0.f, 0.f}};

  for (const auto &charge : charges) {
    bounding_square += charge.bounding_square();
  }

  bounding_square += probe.bounding_square();

  return bounding_square;
}

float calculate_zoom(
    std::span<Charge> charges,
    const Probe &probe,
    const raylib::Vector2 screen_size
) {
  auto bounding_square = world_bounding_square(charges, probe);
  auto bounding_size = world_to_screen(bounding_square.size) * 2.f;
  return std::min(
      screen_size.x / bounding_size.x, screen_size.y / bounding_size.y
  );
}

raylib::Vector2 get_mouse_in_world(raylib::Camera2D camera) {
  return screen_to_world(camera.GetScreenToWorld(raylib::Mouse::GetPosition()));
}

void update_background(
    raylib::Image &background_image,
    raylib::Texture &background_texture,
    const float zoom,
    const raylib::Vector2 world_center,
    const std::span<Charge> charges
) {
  // SAFETY: the image is internally an array of raylib::Colors, so it's
  // safe to treat it as such
  auto background_pixels = std::span(
      static_cast<raylib::Color *>(background_image.data),
      background_image.width * background_image.height
  );

  // SAFETY: each thread will access independent portion of the image
  parallel::for_each<raylib::Color>(
      background_pixels,
      [&background_image, &charges, zoom, world_center](
          const auto i, auto &pixel
      ) {
        auto x = i % background_image.width;
        auto y = i / background_image.width;

        auto x_screen = static_cast<float>(BACKGROUND_SUBSAMPLING * x);
        auto y_screen = static_cast<float>(BACKGROUND_SUBSAMPLING * y);
        auto position =
            screen_to_world(raylib::Vector2{x_screen, y_screen}, zoom) +
            world_center;

        auto potencial = field::potential(position, charges) / 2.f;

        auto normalized = sigmoid(potencial);

        pixel = lerpColor3(
            Charge::NEGATIVE,
            raylib::Color::Black(),
            Charge::POSITIVE,
            normalized
        );
      }
  );

  background_texture.Update(background_image.data);
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
          raylib::Vector2{0, 0}, 1.f, PI / 6.f
      ),
      raylib::Color::Green()
  );

  std::vector<raylib::Color> user_probe_colors{
      raylib::Color::Yellow(),
      raylib::Color::Purple(),
      raylib::Color::Magenta(),
      raylib::Color::SkyBlue(),
      raylib::Color::Brown(),
      raylib::Color::Red(),
  };
  std::vector<std::optional<Probe>> user_probes{};

  auto screen_size = raylib::Vector2(SCREEN_WIDTH, SCREEN_HEIGHT);
  auto half_screen_size = screen_size / 2.f;
  auto half_world_size = screen_to_world(half_screen_size);

  Grid grid(
      screen_size, grid_spacing, raylib::Color::RayWhite(), {150, 150, 150, 255}
  );
  grid.resize(screen_size, grid_spacing);

  raylib::Color textColor(raylib::Color::LightGray());
  raylib::Window w(
      SCREEN_WIDTH,
      SCREEN_HEIGHT,
      "ELEKTROVIZ - A simple visualizer of an electric field",
      FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE
  );

  w.SetTargetFPS(60);

  auto zoom_modifier = 1.f;
  raylib::Camera2D camera(
      half_screen_size, {}, 0.f, calculate_zoom(charges, probe, screen_size)
  );

  FieldLines field_lines{LINES_PER_CHARGE};

  raylib::Image background_image = raylib::Image::Color(
      SCREEN_WIDTH / BACKGROUND_SUBSAMPLING,
      SCREEN_HEIGHT / BACKGROUND_SUBSAMPLING,
      raylib::Color::Blank()
  );
  auto background_texture = raylib::Texture2D(background_image);

  auto simulation_speed = 1.f;
  auto time = 0.0;

  Plot plot(
      {screen_size.x * 0.3f, 0.0},
      {screen_size.x * 0.7f, screen_size.y / 4.f},
      raylib::Color::LightGray()
  );

  Charge *selected_charge;

  auto resize_grid = [&] {
    grid.resize(
        screen_size / camera.GetZoom(),
        grid_spacing / camera.GetZoom(),
        camera.GetTarget()
    );
  };

  auto resize_plot = [&] {
    plot.resize(
        {screen_size.x * 0.3f, 0.0}, {screen_size.x * 0.7f, screen_size.y / 4.f}
    );
  };

  auto recalculate_zoom = [&] {
    camera.SetZoom(calculate_zoom(charges, probe, screen_size) * zoom_modifier);
  };

  // Main game loop
  while (!w.ShouldClose()) // Detect window close button or ESC key
  {
    float bottom_edge = static_cast<float>(w.GetHeight() - 45);
    int slow_button_active = GuiButton(
        raylib::Rectangle{5, bottom_edge, 80, 40},
        GuiIconText(ICON_PLAYER_PREVIOUS, "Slow")
    );
    int fast_button_active = GuiButton(
        raylib::Rectangle{95, bottom_edge, 80, 40},
        GuiIconText(ICON_PLAYER_NEXT, "Fast")
    );
    int normal_speed_button_active = GuiButton(
        raylib::Rectangle{185, bottom_edge, 80, 40},
        GuiIconText(ICON_PLAYER_PLAY, "Normal")
    );

    if (slow_button_active && simulation_speed > .25f)
      simulation_speed /= 2;
    if (fast_button_active && simulation_speed < 4.f)
      simulation_speed *= 2;
    if (normal_speed_button_active)
      simulation_speed = 1.f;

    auto button_active =
        fast_button_active || slow_button_active || normal_speed_button_active;

    auto frameTime = w.GetFrameTime() * simulation_speed;
    time += frameTime;

    auto scroll = raylib::Mouse::GetWheelMove();

    if (std::abs(scroll) > 0) {
      zoom_modifier = std::clamp(zoom_modifier + scroll * 0.05f, 0.5f, 3.f);
      recalculate_zoom();
      half_world_size = screen_to_world(half_screen_size, camera.GetZoom());
      resize_grid();
    }

    if (raylib::Mouse::IsButtonDown(MOUSE_BUTTON_MIDDLE)) {
      auto mouse_in_world = get_mouse_in_world(camera);

      if (!selected_charge) {
        selected_charge = &*ranges::find_if(
            charges,
            std::bind(&Charge::contains, placeholders::_1, mouse_in_world)
        );
      }

      if (selected_charge != &*charges.end()) {
        selected_charge->position(mouse_in_world);
      } else {
        selected_charge = nullptr;
      }
    } else {
      selected_charge = nullptr;
    }

    if (raylib::Mouse::IsButtonDown(MOUSE_BUTTON_RIGHT)) {
      auto delta = raylib::Mouse::GetDelta().Scale(-1.0f / camera.zoom);
      camera.SetTarget(
          Vector2ClampValue(Vector2Add(camera.GetTarget(), delta), 0.f, 800.f)
      );
      resize_grid();
    }

    if (raylib::Mouse::IsButtonPressed(MOUSE_BUTTON_LEFT) && !button_active) {
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
      } else {
        auto first_place = user_probes.empty();

        while (user_probe_colors.size() <= user_probes.size())
          user_probe_colors.push_back(generate_random_color());

        user_probes.push_back(Probe{
            std::make_unique<position::Static>(mouse_in_world),
            user_probe_colors[user_probes.size()]
        });

        if (first_place) {
          camera.SetTarget(Vector2Add(
              camera.GetTarget(),
              raylib::Vector2{0.f, -half_screen_size.y / 4.f}
          ));
          resize_grid();
          resize_plot();
        }
      }
    }

    if (w.IsResized()) {
      screen_size = w.GetSize();
      half_screen_size = screen_size / 2.f;

      recalculate_zoom();

      half_world_size = screen_to_world(half_screen_size, camera.GetZoom());

      // Make 0,0 the center of the screen
      camera.SetOffset(half_screen_size);
      camera.SetZoom(camera.GetZoom());

      resize_grid();
      resize_plot();

      background_texture.Unload();

      auto background_size = screen_size / BACKGROUND_SUBSAMPLING;
      background_image.Resize(background_size.x, background_size.y);

      background_texture = raylib::Texture2D(background_image);
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

    // field_lines.update(charges, camera.GetZoom(), camera.GetTarget());

    update_background(
        background_image,
        background_texture,
        camera.GetZoom(),
        -half_world_size + screen_to_world(camera.GetTarget()),
        std::span(charges)
    );

    // Draw
    w.BeginDrawing();

    background_texture.Draw(
        {}, 0.f, static_cast<float>(BACKGROUND_SUBSAMPLING)
    );

    camera.BeginMode();

    grid.draw();
    // field_lines.draw();
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

    raylib::DrawText(
        std::format("Speed: {:.4g}", simulation_speed),
        text_pos_x,
        bottom_edge - FONT_SIZE - 5,
        FONT_SIZE,
        textColor
    );

    w.EndDrawing();
  }

  return 0;
}
