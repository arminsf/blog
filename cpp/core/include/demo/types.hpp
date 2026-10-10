#pragma once

#include <cstdint>

namespace demo {

struct Vec2 {
    float x = 0, y = 0; 

    Vec2 operator+(const Vec2& v) {
        return { x + v.x, y + v.y };
    }

    Vec2 operator-(const Vec2& v) {
        return { x - v.x, y - v.y };
    }
};

struct Vec3 { 
    float x = 0, y = 0, z = 0; 

    Vec3 operator+(const Vec3& v) {
        return { x + v.x, y + v.y, z + v.z };
    }

    Vec3 operator-(const Vec3& v) {
        return { x - v.x, y - v.y, z - v.z };
    }
};

struct Line2 {
    Vec2 a;
    Vec2 b;
};

struct Line3 {
    Vec3 a;
    Vec3 b;
};

struct Color { std::uint8_t r = 0, g = 0, b = 0, a = 255; };

}