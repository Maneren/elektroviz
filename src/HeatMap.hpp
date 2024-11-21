#pragma once
#include <Image.hpp>
#include <Texture.hpp>
#include <functional>

class HeatMap {
public:
  HeatMap(
      raylib::Vector2 position,
      raylib::Vector2 size,
      raylib::Color max_color,
      raylib::Color mid_color,
      raylib::Color min_color,
      float scale = 1.f
  )
      : position(position), size(size), scale(scale), max_color(max_color),
        mid_color(mid_color), min_color(min_color),
        image{
            static_cast<int>(size.x / scale),
            static_cast<int>(size.y / scale),
            raylib::Color::Black()
        },
        texture{image} {};

  void update(std::function<float(int x, int y)> pixel_function);

  void draw() const;

  void resize(raylib::Vector2 new_size);

private:
  raylib::Vector2 position;
  raylib::Vector2 size;
  float scale;

  raylib::Color max_color;
  raylib::Color mid_color;
  raylib::Color min_color;

  raylib::Image image;
  raylib::Texture2D texture;
};
