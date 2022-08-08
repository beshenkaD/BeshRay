template <typename T> constexpr Vec2<T> operator-(const Vec2<T> &right) { return Vec2<T>(-right.x, -right.y); }

template <typename T> constexpr Vec2<T> &operator+=(Vec2<T> &left, const Vec2<T> &right)
{
    left.x += right.x;
    left.y += right.y;

    return left;
}

////////////////////////////////////////////////////////////
template <typename T> constexpr Vec2<T> &operator-=(Vec2<T> &left, const Vec2<T> &right)
{
    left.x -= right.x;
    left.y -= right.y;

    return left;
}

////////////////////////////////////////////////////////////
template <typename T> constexpr Vec2<T> operator+(const Vec2<T> &left, const Vec2<T> &right)
{
    return Vec2<T>(left.x + right.x, left.y + right.y);
}

////////////////////////////////////////////////////////////
template <typename T> constexpr Vec2<T> operator-(const Vec2<T> &left, const Vec2<T> &right)
{
    return Vec2<T>(left.x - right.x, left.y - right.y);
}

////////////////////////////////////////////////////////////
template <typename T> constexpr Vec2<T> operator*(const Vec2<T> &left, T right)
{
    return Vec2<T>(left.x * right, left.y * right);
}

////////////////////////////////////////////////////////////
template <typename T> constexpr Vec2<T> operator*(T left, const Vec2<T> &right)
{
    return Vec2<T>(right.x * left, right.y * left);
}

////////////////////////////////////////////////////////////
template <typename T> constexpr Vec2<T> &operator*=(Vec2<T> &left, T right)
{
    left.x *= right;
    left.y *= right;

    return left;
}

////////////////////////////////////////////////////////////
template <typename T> constexpr Vec2<T> operator/(const Vec2<T> &left, T right)
{
    return Vec2<T>(left.x / right, left.y / right);
}

////////////////////////////////////////////////////////////
template <typename T> constexpr Vec2<T> &operator/=(Vec2<T> &left, T right)
{
    left.x /= right;
    left.y /= right;

    return left;
}

////////////////////////////////////////////////////////////
template <typename T> constexpr bool operator==(const Vec2<T> &left, const Vec2<T> &right)
{
    return (left.x == right.x) && (left.y == right.y);
}

////////////////////////////////////////////////////////////
template <typename T> constexpr bool operator!=(const Vec2<T> &left, const Vec2<T> &right)
{
    return (left.x != right.x) || (left.y != right.y);
}