#pragma once
#include <raylib-cpp.hpp>

raylib::Color lerpColor(const raylib::Color a, const raylib::Color b,
                        const float t);

raylib::Color lerpColor3(const raylib::Color a, const raylib::Color b,
                         const raylib::Color c, const float t);

std::string trim(const std::string &str);

bool CheckCollisionPointLine(Vector2 point, Vector2 p1, Vector2 p2,
                             float threshold);
