#pragma once

#include <span>

#include <demo/types.hpp>

namespace demo {

class Canvas {
public:
    virtual ~Canvas() = default;
    virtual Vec2 size() const = 0;

    virtual void fill(Color c) = 0;
    virtual void stroke(Color c) = 0;
    virtual void no_fill() = 0;
    virtual void no_stroke() = 0;
    
    virtual void clear() = 0;
    virtual void line(Vec2 a, Vec2 b) = 0;
    virtual void polyline(std::span<Vec2> coords) = 0;
    virtual void circle(Vec2 center, float radius) = 0;
};

}