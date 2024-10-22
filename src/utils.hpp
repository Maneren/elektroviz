#pragma once
#include <Color.hpp>
#include <string>

raylib::Color lerpColor(const raylib::Color a, const raylib::Color b,
                        const float t);

raylib::Color lerpColor3(const raylib::Color a, const raylib::Color b,
                         const raylib::Color c, const float t);

float sigmoid(float x);

std::string trim(const std::string &str);
