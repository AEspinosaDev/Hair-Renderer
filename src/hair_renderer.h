#ifndef __HAIR_RENDERER__
#define __HAIR_RENDERER__

#include <filesystem>
#include <unistd.h>
#include "engine/shader.h"
#include "engine/mesh.h"
#include "engine/loaders.h"
#include "engine/controller.h"
#include "engine/camera.h"
#include "engine/renderer.h"

struct UserInterfaceStyle
{
    ImVec2 padding;
    float windowBorder;
    float windowRounding;
};

class HairRenderer : public Renderer
{
private:
    Camera *m_camera;
    Controller *m_controller;
    Shader *m_shader;
    Mesh *m_mesh;

    bool m_showUI{true};

    void init();
    void update();
    void draw();
    void setup_user_interface_frame();
    void setup_window_callbacks();

#pragma region input
    void key_callback(GLFWwindow *w, int a, int b, int c, int d)
    {
        if (glfwGetKey(m_window.ptr, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(m_window.ptr, true);
        }

        if (glfwGetKey(m_window.ptr, GLFW_KEY_M) == GLFW_PRESS)
        {
            m_showUI = m_showUI ? false : true;
        }
    }
    void mouse_callback(GLFWwindow *w, double x, double y)
    {
        if (!user_interface_wants_to_handle_input())
            m_controller->handle_mouse(w, x, y);
    }
    void resize_callback(GLFWwindow *w, int width, int height)
    {
        m_camera->set_projection(width, height);
        resize(0, 0, width, height);
    }

#pragma endregion

public:
    HairRenderer(const char *title) : Renderer(title) {}
};

#endif
