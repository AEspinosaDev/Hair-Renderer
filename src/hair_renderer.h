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

class HairRenderer : public Renderer
{
private:
    Camera *m_camera;
    Controller *m_controller;

    Shader *m_shader;
    Mesh *m_mesh;

    void init();
    void update();
    void draw();

#pragma region input
    void key_callback(GLFWwindow *w, int a, int b, int c, int d)
    {
        if (glfwGetKey(m_window.ptr, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwWindowShouldClose(m_window.ptr);
        }

    }
    void mouse_callback(GLFWwindow *w, double x, double y)
    {
        // std::cout << m_camera->get_position().b << std::endl;
        m_controller->handle_mouse(w, x, y);
    }
    void resize_callback(GLFWwindow *w, int width, int height)
    {
        m_camera->set_projection(width,height);
        resize(0,0,width,height);
    }

#pragma endregion

public:
    HairRenderer(const char *title) : Renderer(title) {}
};

#endif
