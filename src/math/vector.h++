#pragma once

#include <SFML/System/Vector2.hpp>
#include <cmath>
#include <utility>

namespace beshray {

template <typename T>
struct Vec2 {
    T x = 0, y = 0;

    constexpr Vec2() : x(0), y(0){};
    constexpr Vec2(T x_, T y_) : x(x_), y(y_){};

    template <typename U>
    constexpr explicit Vec2(const Vec2<U> &v) : x(static_cast<T>(v.x)), y(static_cast<T>(v.y))
    {
    }

    [[nodiscard]] constexpr Vec2 rotate(T angle) const
    {
        return {x * std::cos(angle) - y * std::sin(angle), x * std::sin(angle) + y * std::cos(angle)};
    }

    [[nodiscard]] constexpr T len() const { return std::sqrt(x * x + y * y); }

    [[nodiscard]] constexpr T len2() const { return x * x + y * y; }

    [[nodiscard]] constexpr T dot(const Vec2 &v) const { return x * v.x + y * v.y; }

    [[nodiscard]] constexpr T angle() const { return std::atan2(y, x); }

    [[nodiscard]] constexpr Vec2 norm() const
    {
        T s = 1 / len();
        return {x * s, y * s};
    }

    [[nodiscard]] constexpr Vec2 perp() const { return {-y, x}; }

    constexpr operator sf::Vector2<T>() const { return {x, y}; }
};

using Vec2i = Vec2<int>;
using Vec2f = Vec2<float>;
using Vec2u = Vec2<unsigned>;

template <typename T>
[[nodiscard]] constexpr Vec2<T> operator-(const Vec2<T> &right);

template <typename T>
constexpr Vec2<T> &operator+=(Vec2<T> &left, const Vec2<T> &right);

template <typename T>
constexpr Vec2<T> &operator-=(Vec2<T> &left, const Vec2<T> &right);

template <typename T>
[[nodiscard]] constexpr Vec2<T> operator+(const Vec2<T> &left, const Vec2<T> &right);

template <typename T>
[[nodiscard]] constexpr Vec2<T> operator-(const Vec2<T> &left, const Vec2<T> &right);

template <typename T>
[[nodiscard]] constexpr Vec2<T> operator*(const Vec2<T> &left, T right);

template <typename T>
[[nodiscard]] constexpr Vec2<T> operator*(T left, const Vec2<T> &right);

template <typename T>
constexpr Vec2<T> &operator*=(Vec2<T> &left, T right);

template <typename T>
[[nodiscard]] constexpr Vec2<T> operator/(const Vec2<T> &left, T right);

template <typename T>
constexpr Vec2<T> &operator/=(Vec2<T> &left, T right);

template <typename T>
[[nodiscard]] constexpr bool operator==(const Vec2<T> &left, const Vec2<T> &right);

template <typename T>
[[nodiscard]] constexpr bool operator!=(const Vec2<T> &left, const Vec2<T> &right);

#include "vector.inl"

} // namespace beshray