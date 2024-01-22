#ifndef __RENDERER__
#define __RENDERER__

#include "core.h"

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

    // m_initQueue;
    // m_cleanupQueue;


    virtual void init();
    virtual void create_context();
    virtual void tick();
    virtual void update();
    virtual void draw();
    virtual void cleanup();

public:
    Renderer(const char *title) { m_window.title = title; }
    Renderer(Window &window) { m_window = window; }
    Renderer(Window &window, ContextSettings &contextSettings) : m_window(window), m_context(contextSettings) {}

    void run();
};

#endif