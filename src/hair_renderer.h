#ifndef __HAIR_RENDERER__
#define __HAIR_RENDERER__

#include <filesystem>
#include <unistd.h>
#include <thread>

#include "engine/shader.h"
#include "engine/mesh.h"
#include "engine/loaders.h"
#include "engine/controller.h"
#include "engine/camera.h"
#include "engine/light.h"
#include "engine/gui_layer.h"
#include "engine/uniforms.h"
#include "engine/framebuffer.h"
#include "engine/renderer.h"

#include "settings.h"
#include "hair_loaders.h"

USING_NAMESPACE_GLIB

class HairRenderer : public Renderer
{
private:
    //--- Scene ---

    Camera *m_camera;
    Controller *m_controller;

    Mesh *m_hair;
    Mesh *m_head;

    struct LightData
    {
        PointLight *light{nullptr};
        Mesh *dummy{nullptr};

        void set_position(glm::vec3 p)
        {
            light->set_position(p);
            dummy->set_position(p);
        }
    };

    LightData m_light{};

    //--- Uniform data ---

    enum UBOLayout
    {
        CAMERA_LAYOUT = 0,
        GLOBAL_LAYOUT = 1,
        OBJECT_LAYOUT = 2
    };

    UniformBuffer *m_cameraUBO;
    UniformBuffer *m_globalUBO;
    UniformBuffer *m_objectUBO;

    //--- Framebuffer data ---

    Framebuffer* m_shadowFBO;

    //--- Settings ---

    GlobalSettings m_globalSettings{};
    UserInterfaceSettings m_UISettigns{};
    HairSettings m_hairSettings{};
    HeadSettings m_headSettings{};

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

        if (glfwGetKey(m_window.ptr, GLFW_KEY_F11) == GLFW_PRESS)
        {
            m_window.set_fullscreen(m_window.fullscreen ? false : true);
        }

        if (glfwGetKey(m_window.ptr, GLFW_KEY_M) == GLFW_PRESS)
        {
            // m_showUI = m_showUI ? false : true;
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
