#ifndef __RENDERER__
#define __RENDERER__

#include "core.h"
#include "utils.h"

GLIB_NAMESPACE_BEGIN

struct Window
{
    Extent2D extent{800, 600};
    Position2D position{50, 50};
    const char *title;
    GLFWwindow *ptr{nullptr};
    bool fullscreen{false};

    inline void set_fullscreen(bool op)
    {
        fullscreen = op;
        if (!fullscreen)
        {
            glfwSetWindowMonitor(ptr, NULL, position.x, position.y, extent.width, extent.height, GLFW_DONT_CARE);
        }
        else
        {
            const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            glfwSetWindowMonitor(ptr, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
        }
    }
};

struct ContextSettings
{
    int OpenGLMajor{4};
    int OpenGLMinor{6};
    int OpenGLProfile{GLFW_OPENGL_CORE_PROFILE};
};
struct RendererSettings
{
    bool vSync{true};
    int framerateCap{-1};
    bool userInterface{true};
    bool depthTest{true};
    bool depthWrites{true};
    bool blending{true};
};

class Renderer
{
protected:
    const ContextSettings m_context{};
    RendererSettings m_settings{};

    Window m_window{};

    struct Time
    {
        double delta{0.0};
        double last{0.0};
        double current{0.0};
        int framerate{0};
    };
    Time m_time{};

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
    /*
    Setup GLFW window callbacks here
     */
    virtual void setup_window_callbacks();

public:
    Renderer(const char *title) { m_window.title = title; }
    Renderer(Window &window) { m_window = window; }
    Renderer(Window &window, ContextSettings &contextSettings, RendererSettings &settings) : m_window(window), m_context(contextSettings), m_settings(settings) {}

    void run();

#pragma region getters&setters
    inline RendererSettings get_settings() const
    {
        return m_settings;
    }
    inline void set_settings(RendererSettings &s)
    {
        m_settings = s;
    }
    inline Time get_time() const
    {
        return m_time;
    }
    inline void set_v_sync(bool op)
    {
        glfwSwapInterval(op);
        m_settings.vSync = op;
    }
#pragma endregion

#pragma region opengl_state_wrappers
    /*
    Use as callback
    */
    inline virtual void resize(int x, int y, int w, int h)
    {
        GL_CHECK(glViewport(0, 0, w, h));
        m_window.extent = {w, h};
    }
    inline virtual void clearColorBit()
    {
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));
    }
    inline virtual void clearColorDepthBit()
    {
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    }
    inline virtual void clearDepthBit()
    {
        GL_CHECK(glClear(GL_DEPTH_BUFFER_BIT));
    }
    inline virtual void enableDepthTest(bool op)
    {
        op ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
        m_settings.depthTest = op;
    }
    inline virtual void enableDepthWrites(bool op)
    {
        GL_CHECK(glDepthMask(op));
        m_settings.depthWrites = op;
    }
#pragma endregion
#pragma region user_interface
    inline bool user_interface_wants_to_handle_input()
    {
        ImGuiIO &io = ImGui::GetIO();
        if (io.WantCaptureMouse || io.WantCaptureKeyboard)
            return true;
        else
            return false;
    }
    /*
    Override if custom init functionality needed. Predetermined graphic user interface backend is IMGUI.
    */
    virtual void init_user_interface();
    /*
    Override to add IMGUI windows and widgets.Call ImGui::NewFrame at start and ImGui::Render at the end. Don't forget to call parent function for IMGUI frame set up
    */
    virtual void setup_user_interface_frame();

    virtual void upload_user_interface_render_data();

#pragma endregion
};
GLIB_NAMESPACE_END
#endif