#ifndef __RENDERER__
#define __RENDERER__

#include "core.h"
#include "utils.h"

struct Extent2D
{
    int width{0};
    int height{0};
};
struct Window
{
    Extent2D extent{800, 600};
    const char *title;
    GLFWwindow *ptr{nullptr};
};

struct ContextSettings
{
    int OpenGLMajor{4};
    int OpenGLMinor{6};
    int OpenGLProfile{GLFW_OPENGL_CORE_PROFILE};
};

class Renderer
{
protected:
    const ContextSettings m_context{};
    Window m_window{};

    struct Time
    {
        double delta{0.0};
        double last{0.0};
        double current{0.0};
        int framesPerSecond{0};
    };
    Time m_time{};

    utils::EventDispatcher m_initQueue;
    utils::EventDispatcher m_cleanupQueue;

    void create_context();
    void tick();
    void cleanup();
    /*
    Override function in order to initiate desired funcitonality. Call parent function if want to use events functionality.
    */
    virtual void init();
     /*
    Override function in order to customize update.
    */
    virtual void update();
     /*
    Override function in order to customize render funcitonality.
    */
    virtual void draw();


public:
    Renderer(const char *title) { m_window.title = title; }
    Renderer(Window &window) { m_window = window; }
    Renderer(Window &window, ContextSettings &contextSettings) : m_window(window), m_context(contextSettings) {}

    void run();
};

#endif