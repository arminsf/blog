#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#ifdef __EMSCRIPTEN__
    #include <GLES3/gl3.h>
#else
    #include <glad/gl.h>
#endif

#include <algorithm>
#include <memory>
#include <vector>
#include <span>
#include <cmath>
#include <fstream>
#include <sstream>

#include <demo/canvas.hpp>
#include <demo/input.hpp>
#include <demo/demo.hpp>

#define PI M_PI

namespace {

class OpenglCanvas : public demo::Canvas {

private:
    unsigned int shaderProgram_;

    unsigned int circleVAO_;
    unsigned int circleVBO_;
    unsigned int lineVAO_;
    unsigned int lineVBO_;

    demo::Vec2 size_;
    demo::Color fill_;
    demo::Color stroke_;
    bool no_fill_ = false;
    bool no_stroke_ = false;

    void initialize_vao(GLuint* vao, GLuint* vbo) {
        glGenVertexArrays(1, vao);
        glGenBuffers(1, vbo);

        glBindVertexArray(*vao);
        glBindBuffer(GL_ARRAY_BUFFER, *vbo);

        glVertexAttribPointer(
            0,
            3,
            GL_FLOAT,
            GL_FALSE,
            3 * sizeof(float),
            (void*)0
        );

        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }

    void clean_vao(GLuint* vao, GLuint* vbo) {
        glDeleteBuffers(1, vbo);
        glDeleteVertexArrays(1, vao);
    }

    void initialize_vaos() {
        initialize_vao(&lineVAO_, &lineVBO_);
        initialize_vao(&circleVAO_, &circleVBO_);
    }

    void clean_vaos() {
        clean_vao(&lineVAO_, &lineVBO_);
        clean_vao(&circleVAO_, &circleVBO_);
    }

    void set_gl_color(demo::Color c) {
        GLint color_loc = glGetUniformLocation(shaderProgram_, "color");
        glUniform4f(color_loc, c.r / 255.0, c.g / 255.0, c.b / 255.0, c.a / 255.0);
    }

public:
    OpenglCanvas(unsigned int shaderProgram, demo::Vec2 size)
       : shaderProgram_(shaderProgram), size_(size) {
        initialize_vaos();
    }

    ~OpenglCanvas() {
        clean_vaos();
    }

    demo::Vec2 size() const override { return size_; }

    void fill(demo::Color c) override { 
        no_fill_ = false; 
        fill_ = c; 
        set_gl_color(fill_);
    }
    void stroke(demo::Color c) override { 
        no_stroke_ = false;
        stroke_ = c;
        set_gl_color(stroke_);
    }
    void no_fill() override { no_fill_ = true; }
    void no_stroke() override { no_stroke_ = true; }

    void clear() override {
        if (no_fill_) return;

        glClearColor(fill_.r / 255.0f, fill_.g / 255.0f, fill_.b / 255.0f, fill_.a / 255.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void line(demo::Vec2 a, demo::Vec2 b) override {
        if (no_stroke_) return;

        float vertices[6];

        vertices[0] = a.x;
        vertices[1] = a.y;
        vertices[2] = 0.0f;

        vertices[3] = b.x;
        vertices[4] = b.y;
        vertices[5] = 0.0f;

        for (int i = 0; i < 2; i++) {
            vertices[3*i] -= 0.5f * size_.x;
            vertices[3*i+1] -= 0.5f * size_.y;
            vertices[3*i] *= 2.0f / size_.x;
            vertices[3*i+1] *= -2.0f / size_.y;
        }

        glBindVertexArray(lineVAO_);
        glBindBuffer(GL_ARRAY_BUFFER, lineVBO_);

        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(vertices),
            vertices,
            GL_DYNAMIC_DRAW
        );

        glUseProgram(shaderProgram_);

        glDrawArrays(GL_LINES, 0, 2);
    }

    void polyline(std::span<demo::Vec2> coords) override {
        if (no_stroke_) return;

        const std::size_t n = coords.size();

        std::vector<float> vertices(3 * n, 0.0);

        for (std::size_t i = 0; i < n; i++) {
            vertices[3*i] = coords[i].x;
            vertices[3*i+1] = coords[i].y;
            vertices[3*i+2] = 0.0f;

            vertices[3*i] -= 0.5f * size_.x;
            vertices[3*i+1] -= 0.5f * size_.y;
            vertices[3*i] *= 2.0f / size_.x;
            vertices[3*i+1] *= -2.0f / size_.y;
        }

        glBindVertexArray(lineVAO_);
        glBindBuffer(GL_ARRAY_BUFFER, lineVBO_);

        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(float) * 3 * n,
            vertices.data(),
            GL_DYNAMIC_DRAW
        );

        glUseProgram(shaderProgram_);

        glDrawArrays(GL_LINES, 0, n);
    }

    void circle(demo::Vec2 center, float radius) override {
        if (no_fill_ && no_stroke_) return;

        const int ntriangles = 60;
        float vertices[3 * (2 + ntriangles)];

        vertices[0] = center.x;
        vertices[1] = center.y;
        vertices[2] = 0.0f;

        for (int i = 1; i <= ntriangles + 1; i++) {
            vertices[3*i] =
                center.x + radius * std::cos(i * 2 * PI / ntriangles);

            vertices[3*i+1] =
                center.y + radius * std::sin(i * 2 * PI / ntriangles);

            vertices[3*i+2] = 0.5f;
        }

        for (int i = 0; i <= ntriangles + 1; i++) {
            vertices[3*i] -= 0.5f * size_.x;
            vertices[3*i+1] -= 0.5f * size_.y;
            vertices[3*i] *= 2.0f / size_.x;
            vertices[3*i+1] *= -2.0f / size_.y;
        }

        glBindVertexArray(circleVAO_);
        glBindBuffer(GL_ARRAY_BUFFER, circleVBO_);

        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(vertices),
            vertices,
            GL_DYNAMIC_DRAW
        );

        glUseProgram(shaderProgram_);

        if (!no_fill_) {
            set_gl_color(fill_);
            glDrawArrays(GL_TRIANGLE_FAN, 0, ntriangles+2);
        }

        if (!no_stroke_) {
            set_gl_color(stroke_);
            glDrawArrays(GL_LINE_LOOP, 1, ntriangles+1);
        }
    }
};

struct HostData {
    SDL_Window* window = nullptr;
    SDL_GLContext gl_context;

    unsigned int vertexShader;
    unsigned int fragmentShader;
    unsigned int shaderProgram;

    std::unique_ptr<OpenglCanvas> canvas;
    std::unique_ptr<demo::Demo> demo; // fetched from create_demo()
    demo::InputData inputdata;
    Uint64 last_ns;
};

}

std::string load_shader(const std::string& filename) {
    std::ifstream file;
    file.open(filename);
    std::stringstream str;
    str << file.rdbuf();
    return str.str();
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

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);   // ES 3.0 ≈ WebGL2
    #else
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    #endif

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    // glEnable(GL_MULTISAMPLE);
    // glEnable(GL_LINE_SMOOTH);

    hostdata->window = SDL_CreateWindow(title, int(size.x), int(size.y), SDL_WINDOW_OPENGL);
    hostdata->gl_context = SDL_GL_CreateContext(hostdata->window);
    SDL_GL_SetSwapInterval(1);

    #ifndef __EMSCRIPTEN__
        if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress)) {
            SDL_Log("%s", "gladLoadGL failed");
            return SDL_APP_FAILURE;
        }
    #endif
    
    int  success;
    char infoLog[512];

    const std::string vss = load_shader(std::string(SHADER_SRC_DIR) + "/basic.vert");
    const std::string fss = load_shader(std::string(SHADER_SRC_DIR) + "/basic.frag");
    const char* const vertexShaderSource = vss.c_str();
    const char* const fragmentShaderSource = fss.c_str();

    hostdata->vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(hostdata->vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(hostdata->vertexShader);
    glGetShaderiv(hostdata->vertexShader, GL_COMPILE_STATUS, &success);
    
    if(!success)
    {
        glGetShaderInfoLog(hostdata->vertexShader, 512, NULL, infoLog);
        SDL_Log("%s", infoLog);
    }

    hostdata->fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(hostdata->fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(hostdata->fragmentShader);
    glGetShaderiv(hostdata->fragmentShader, GL_COMPILE_STATUS, &success);
    
    if(!success)
    {
        glGetShaderInfoLog(hostdata->fragmentShader, 512, NULL, infoLog);
        SDL_Log("%s", infoLog);
    }

    hostdata->shaderProgram = glCreateProgram();
    glAttachShader(hostdata->shaderProgram, hostdata->vertexShader);
    glAttachShader(hostdata->shaderProgram, hostdata->fragmentShader);
    glLinkProgram(hostdata->shaderProgram);
    glGetProgramiv(hostdata->shaderProgram, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(hostdata->shaderProgram, 512, NULL, infoLog);
        SDL_Log("%s", infoLog);
    }

    hostdata->canvas = std::make_unique<OpenglCanvas>(hostdata->shaderProgram, size);
    hostdata->last_ns = SDL_GetTicksNS();
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* state, SDL_AppResult) {
    auto* hostdata = static_cast<HostData*>(state);
    if (!hostdata) return;
    
    glDeleteShader(hostdata->vertexShader);
    glDeleteShader(hostdata->fragmentShader);
    glDeleteProgram(hostdata->shaderProgram);
    if (hostdata->window) SDL_DestroyWindow(hostdata->window);

    delete hostdata;
}

SDL_AppResult SDL_AppIterate(void* state) {
    HostData* hostdata = static_cast<HostData*>(state);

    const Uint64 now = SDL_GetTicksNS();
    double dt = std::min(double(now - hostdata->last_ns) / 1e9, 0.1); // converts to seconds
    hostdata->last_ns = now;

    hostdata->demo->update(dt, hostdata->inputdata);
    hostdata->demo->draw(*hostdata->canvas);
    
    SDL_GL_SwapWindow(hostdata->window);
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