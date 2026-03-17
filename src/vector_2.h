//
// Created by jhone on 16/03/2026.
//

#ifndef VECTOR_2_H
#define VECTOR_2_H

#include <cmath>

class Vector2 {

public:
    float x;
    float y;

    Vector2() : x(0.0f), y(0.0f) {}
    Vector2(float x, float y) : x(x), y(y) {}

    static Vector2 zero() { return {0, 0}; }
    static Vector2 one() { return {1, 1}; }
    static Vector2 up() { return {0, 1}; }
    static Vector2 down() { return {0, -1}; }
    static Vector2 left() { return {-1, 0}; }
    static Vector2 right() { return {1, 0}; }

    float length() const {
        return std::sqrt(x * x + y * y);
    }

    float lengthSquared() const {
        return x * x + y * y;
    }

    Vector2 normalized() const {
        float len = length();
        if (len == 0) return {0,0};
        return {x / len, y / len};
    }

    void normalize() {
        float len = length();
        if (len == 0) return;
        x /= len;
        y /= len;
    }

    float dot(const Vector2& other) const {
        return x * other.x + y * other.y;
    }

    static float distance(const Vector2& a, const Vector2& b) {
        return (a - b).length();
    }

    static Vector2 lerp(const Vector2& a, const Vector2& b, float t) {
        return a + (b - a) * t;
    }

    static float angle(const Vector2& a, const Vector2& b) {
        float dotVal = a.dot(b);
        float len = a.length() * b.length();
        if (len == 0) return 0;
        return std::acos(dotVal / len);
    }

    Vector2 rotated(float radians) const {
        float cosA = std::cos(radians);
        float sinA = std::sin(radians);

        return Vector2(
            x * cosA - y * sinA,
            x * sinA + y * cosA
        );
    }

    Vector2 operator+(const Vector2& other) const {
        return Vector2(x + other.x, y + other.y);
    }

    Vector2 operator-(const Vector2& other) const {
        return Vector2(x - other.x, y - other.y);
    }

    Vector2 operator*(float scalar) const {
        return Vector2(x * scalar, y * scalar);
    }

    Vector2 operator/(float scalar) const {
        return Vector2(x / scalar, y / scalar);
    }

    Vector2& operator+=(const Vector2& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vector2& operator-=(const Vector2& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Vector2& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    Vector2& operator/=(float scalar) {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    bool operator==(const Vector2& other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Vector2& other) const {
        return !(*this == other);
    }
};



#endif //VECTOR_2_H
