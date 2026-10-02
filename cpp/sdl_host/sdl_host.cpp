#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <algorithm>
#include <memory>
#include <cmath>

#include <demo/canvas.hpp>
#include <demo/input.hpp>
#include <demo/demo.hpp>

namespace {

class SdlCanvas : public demo::Canvas {

private:
    SDL_Renderer* renderer_;
    demo::Vec2 size_;
    demo::Color fill_;
    demo::Color stroke_;
    bool no_fill_ = false;
    bool no_stroke_ = false;

public:
    SdlCanvas(SDL_Renderer* r, demo::Vec2 size)
        : renderer_(r), size_(size) {}

    demo::Vec2 size() const override { return size_; }

    void fill(demo::Color c) override { no_fill_ = false; fill_ = c; }
    void stroke(demo::Color c) override { no_stroke_ = false; stroke_ = c; }
    void no_fill() override { no_fill_ = true; }
    void no_stroke() override { no_stroke_ = true; }

    void clear() override {
        if (no_fill_) return;
        SDL_SetRenderDrawColor(renderer_, fill_.r, fill_.g, fill_.b, fill_.a);
        SDL_RenderClear(renderer_);
    }

    void line(demo::Vec2 a, demo::Vec2 b) override {
        if (no_stroke_) return;
        SDL_SetRenderDrawColor(renderer_, stroke_.r, stroke_.g, stroke_.b, stroke_.a);
        SDL_RenderLine(renderer_, a.x, a.y, b.x, b.y);
    }

    void polyline(std::span<demo::Vec2> coords) override {
        if (no_stroke_) return;
        SDL_SetRenderDrawColor(renderer_, stroke_.r, stroke_.g, stroke_.b, stroke_.a);
        for (std::size_t i = 0; i < coords.size(); i++) {
            SDL_RenderLine(
                renderer_,
                coords[i].x,
                coords[i].y,
                coords[i+1].x,
                coords[i+1].y
            );
        }
    }

    void circle(demo::Vec2 center, float radius) override {
        SDL_SetRenderDrawColor(renderer_, fill_.r, fill_.g, fill_.b, fill_.a);
        const int r = static_cast<int>(radius);
        for (int dy = -r; dy <= r; ++dy) {
            float half = std::sqrt(radius * radius - float(dy * dy));
            SDL_RenderLine(renderer_,
                           center.x - half, center.y + dy,
                           center.x + half, center.y + dy);
        }
    }
};

struct HostData {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    std::unique_ptr<SdlCanvas> canvas;
    std::unique_ptr<demo::Demo> demo; // fetched from create_demo()
    demo::InputData inputdata;
    Uint64 last_ns;
};

}

SDL_AppResult SDL_AppInit(void** state, int argc, char** argv) {
    HostData* hostdata = new HostData;
    *state = hostdata;

    hostdata->demo = demo::create_demo(); // this is how we'll call the demo-specific update functions;
    const demo::Vec2 size = hostdata->demo->preferred_size();
    const char* title = hostdata->demo->title();

    if (!SDL_Init(SDL_INIT_VIDEO))
        return SDL_APP_FAILURE;

    #ifdef __EMSCRIPTEN__
    const char* canvas_selector = (argc > 1) ? argv[1] : "#canvas";
    SDL_SetHint(SDL_HINT_EMSCRIPTEN_CANVAS_SELECTOR, canvas_selector);
    SDL_SetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT, canvas_selector + 1);
    #endif

    if (!SDL_CreateWindowAndRenderer(title, int(size.x), int(size.y), 0, &hostdata->window, &hostdata->renderer))
        return SDL_APP_FAILURE;

    hostdata->canvas = std::make_unique<SdlCanvas>(hostdata->renderer, size);
    hostdata->last_ns = SDL_GetTicksNS();
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* state, SDL_AppResult) {
    auto* hostdata = static_cast<HostData*>(state);
    if (!hostdata) return;
    
    if (hostdata->renderer) SDL_DestroyRenderer(hostdata->renderer);
    if (hostdata->window)   SDL_DestroyWindow(hostdata->window);

    delete hostdata;
}

SDL_AppResult SDL_AppIterate(void* state) {
    HostData* hostdata = static_cast<HostData*>(state);

    const Uint64 now = SDL_GetTicksNS();
    double dt = std::min(double(now - hostdata->last_ns) / 1e9, 0.1);
    hostdata->last_ns = now;

    hostdata->demo->update(dt, hostdata->inputdata);
    hostdata->demo->draw(*hostdata->canvas);
    
    SDL_RenderPresent(hostdata->renderer);
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* state, SDL_Event* e) {
    HostData* hostdata = static_cast<HostData*>(state);
    switch (e->type) {
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;
        case SDL_EVENT_MOUSE_MOTION:
            hostdata->inputdata.pointer = {e->motion.x, e->motion.y};
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
            if (e->button.button == SDL_BUTTON_LEFT)
                hostdata->inputdata.pointer_held =
                    (e->type == SDL_EVENT_MOUSE_BUTTON_DOWN);
            break;
    }
    return SDL_APP_CONTINUE;
}