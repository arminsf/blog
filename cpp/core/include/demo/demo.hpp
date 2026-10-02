#pragma once

#include <memory>
#include <cstdint>

#include <demo/canvas.hpp>
#include <demo/types.hpp>
#include <demo/input.hpp>

namespace demo {

class Demo {
public:
    virtual ~Demo() = default;
    virtual Vec2 preferred_size() const { return {640, 360}; };
    virtual const char* title() const { return "demo"; };
    virtual void update(float dt, const InputData& input) = 0;
    virtual void draw(Canvas& canvas) = 0;
};

std::unique_ptr<Demo> create_demo(); // called from SDL code later

} // namespace demo