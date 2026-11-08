/*
** EPITECH PROJECT, 2026
** r-type_engine
** File description:
** Vec2
*/

#ifndef ENGINE_VEC2_HPP
#define ENGINE_VEC2_HPP

namespace engine {

/**
 * @struct Vec2
 * @brief A two-dimensional vector or position.
 * @tparam T The type of the coordinates.
 * @param x The horizontal coordinate.
 * @param y The vertical coordinate.
 **/
template <typename T> struct Vec2 {
    T x{};
    T y{};

    /**
     * @brief Adds a vector to this one.
     * @param other The vector to add.
     * @return This vector.
     **/
    constexpr Vec2 &operator+=(Vec2 other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    /**
     * @brief Subtracts a vector from this one.
     * @param other The vector to subtract.
     * @return This vector.
     **/
    constexpr Vec2 &operator-=(Vec2 other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    /**
     * @brief Multiplies both coordinates by a number.
     * @param scalar The number.
     * @return This vector.
     **/
    constexpr Vec2 &operator*=(T scalar)
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    /**
     * @brief Adds two vectors.
     * @param a The first vector.
     * @param b The second vector.
     * @return Their sum.
     **/
    friend constexpr Vec2 operator+(Vec2 a, Vec2 b) { return a += b; }
    /**
     * @brief Subtracts a vector from another.
     * @param a The first vector.
     * @param b The vector to subtract from it.
     * @return Their difference.
     **/
    friend constexpr Vec2 operator-(Vec2 a, Vec2 b) { return a -= b; }
    /**
     * @brief Multiplies a vector by a number.
     * @param vec The vector.
     * @param scalar The number.
     * @return The scaled vector.
     **/
    friend constexpr Vec2 operator*(Vec2 vec, T scalar)
    {
        return vec *= scalar;
    }
    /**
     * @brief Multiplies a vector by a number.
     * @param scalar The number.
     * @param vec The vector.
     * @return The scaled vector.
     **/
    friend constexpr Vec2 operator*(T scalar, Vec2 vec)
    {
        return vec *= scalar;
    }
    friend constexpr bool operator==(Vec2, Vec2) = default;
};

/** @brief A vector in playfield units or pixels, as floats. */
using Vec2f = Vec2<float>;

} // namespace engine

#endif // ENGINE_VEC2_HPP
