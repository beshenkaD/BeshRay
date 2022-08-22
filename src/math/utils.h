#pragma once

#include <numbers>

namespace beshray::utils {

constexpr float radToDeg(const float angle) { return angle * (180 / std::numbers::pi); }
constexpr float degToRad(const float angle) { return angle * (std::numbers::pi / 180); }

} // namespace beshray::utils