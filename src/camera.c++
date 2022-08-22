#include "camera.h++"
#include "math/utils.h"

namespace beshray {

void Camera::setFOV(const int degrees)
{
    float d = 1 / std::tan(beshray::utils::degToRad(degrees / 2));
    float s = d / dir.len();

    dir *= s;
}

void Camera::setFOV(const float radians)
{
    float d = 1 / std::tan(radians / 2);
    float s = d / dir.len();

    dir *= s;
}

} // namespace beshray