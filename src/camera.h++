#pragma once

#include "math/vector.h++"
#include <cmath>

namespace beshray {

class Camera {
  public:
    Vec2f pos = {0, 0};
    float pitch = 0;
    float height = 0;

    Camera(const float x, const float y, int FOV = 90, const float p = 0, const float h = 0) : pos{x, y}, pitch(p), height(h)
    {
        setFOV(FOV);
    }
    Camera() = default;

    void move(const float speed) { pos += dir * speed; };
    void tilt(const float speed) { pitch += speed; };
    void lift(const float speed) { height += speed; };
    void rotate(const float angle)
    {
        dir = dir.rotate(angle);
        plane = plane.rotate(angle);
    };

    [[nodiscard]] float getHeading() const { return 2 * std::atan(plane.len() / dir.len()); }
    void setFOV(const int degrees);
    void setFOV(const float radians);

  private:
    Vec2f dir = {-1, 0};
    Vec2f plane = {0, 1};

    friend class Engine;
};

} // namespace beshray