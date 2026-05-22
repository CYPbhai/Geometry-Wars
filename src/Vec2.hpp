#pragma once

#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>

template <typename T>
class Vec2
{
public:
    T x = 0;
    T y = 0;

    Vec2() = default;
    Vec2(T xin, T yin) : x(xin), y(yin) {}

    Vec2(const sf::Vector2<T>& vec) : x(vec.x), y(vec.y) {}

    operator sf::Vector2<T>() const { return sf::Vector2<T>(x, y); }

    Vec2 operator+(const Vec2& rhs) const { return Vec2(x + rhs.x, y + rhs.y); }
    Vec2 operator-(const Vec2& rhs) const { return Vec2(x - rhs.x, y - rhs.y); }
    Vec2 operator/(const Vec2& rhs) const { return Vec2(x / rhs.x, y / rhs.y); }
    Vec2 operator*(const Vec2& rhs) const { return Vec2(x * rhs.x, y * rhs.y); }
	Vec2 operator*(float scalar) const { return Vec2(x * scalar, y * scalar); }

    bool operator==(const Vec2& rhs) const { return (x == rhs.x) && (y == rhs.y); }
    bool operator!=(const Vec2& rhs) const { return !(*this == rhs); }

    void operator+=(const Vec2& rhs) { x += rhs.x; y += rhs.y; }
    void operator-=(const Vec2& rhs) { x -= rhs.x; y -= rhs.y; }
    void operator/=(const Vec2& rhs) { x /= rhs.x; y /= rhs.y; }
    void operator*=(const Vec2& rhs) { x *= rhs.x; y *= rhs.y; }

    float distanceSquaredTo(const Vec2& rhs) const
    {
        return (x - rhs.x) * (x - rhs.x) + (y - rhs.y) * (y - rhs.y);
    }

    float distanceTo(const Vec2& rhs) const { return std::sqrt(distanceSquaredTo(rhs)); }

    float length() const { return std::sqrt(x * x + y * y); }

    void normalize()
    {
        float len = length();
        if (len != 0)
        {
            x /= len;
            y /= len;
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const Vec2& v)
    {
        os << "Vec2(" << v.x << ", " << v.y << ")";
        return os;
    }
};

using Vec2f = Vec2<float>;
using Vec2i = Vec2<int>;
