#include "HeatMap.hpp"
#include "parallel.hpp"
#include "utils.hpp"

void HeatMap::update(std::function<float(size_t x, size_t y)> &&pixel_function
) {
  auto pixels = std::span(
      static_cast<raylib::Color *>(image.data), image.width * image.height
  );

  parallel::for_each<raylib::Color>(
      pixels,
      [this, &pixel_function](const auto i, auto &pixel) {
        auto x = i % image.width;
        auto y = i / image.width;
        auto value = pixel_function(x, y);
        pixel = lerpColor3(min_color, mid_color, max_color, value);
      }
  );

  texture.Update(image.data);
}

void HeatMap::draw() const { texture.Draw(position, 0.f, scale); }

void HeatMap::resize(raylib::Vector2 new_size) {
  texture.Unload();
  image.Resize(new_size.x, new_size.y);
  texture = raylib::Texture2D(image);
}
