/*
** EPITECH PROJECT, 2026
** r-type_engine
** File description:
** Vec2
*/

#ifndef ENGINE_VEC2_HPP
#define ENGINE_VEC2_HPP

namespace engine {

template <typename T> struct Vec2 {
    T x{};
    T y{};

    constexpr Vec2 &operator+=(Vec2 other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    constexpr Vec2 &operator-=(Vec2 other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    constexpr Vec2 &operator*=(T scalar)
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    friend constexpr Vec2 operator+(Vec2 a, Vec2 b) { return a += b; }
    friend constexpr Vec2 operator-(Vec2 a, Vec2 b) { return a -= b; }
    friend constexpr Vec2 operator*(Vec2 vec, T scalar)
    {
        return vec *= scalar;
    }
    friend constexpr Vec2 operator*(T scalar, Vec2 vec)
    {
        return vec *= scalar;
    }
    friend constexpr bool operator==(Vec2, Vec2) = default;
};

using Vec2f = Vec2<float>;

} // namespace engine

#endif // ENGINE_VEC2_HPP
