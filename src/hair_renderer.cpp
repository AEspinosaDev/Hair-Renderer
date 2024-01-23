#include "hair_renderer.h"

void HairRenderer::init()
{
    Renderer::init();

    glfwSetWindowUserPointer(m_window.ptr, this);
    auto keyCallback = [](GLFWwindow *w, int key, int scancode, int action, int mods)
    {
         static_cast<HairRenderer *>(glfwGetWindowUserPointer(w))->key_callback(w,key,scancode,action,mods);
    };
    auto mouseCallback = [](GLFWwindow *w, double x, double y)
    {
        static_cast<HairRenderer *>(glfwGetWindowUserPointer(w))->mouse_callback(w,x,y);
    };

    glfwSetKeyCallback(m_window.ptr, keyCallback);
    glfwSetCursorPosCallback(m_window.ptr, mouseCallback);

    chdir("/home/tony/Dev/OpenGL-Hair/");

    m_camera = new Camera();
    m_controller = new Controller(m_camera);

    m_shader = new Shader("resources/shaders/test.glsl", UNLIT);

    m_mesh = new Mesh();
    Geometry g;
    g.vertices = {{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                  {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                  {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
                  {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}}};

    g.indices = {0, 1, 2, 2, 3, 0};

    // m_mesh->set_geometry(g);
    OBJ_loader::load_mesh(m_mesh, false, "resources/models/cube.obj");

    m_mesh->generate_buffers();

    glDisable(GL_CULL_FACE);
}

void HairRenderer::update()
{
    m_controller->handle_keyboard(m_window.ptr, 0, 0, m_time.delta);
}

void HairRenderer::draw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_shader->bind();
    m_camera->set_projection(800, 600);
    m_shader->set_mat4("u_viewProj", m_camera->get_projection() * m_camera->get_view());
    m_shader->set_mat4("u_model", m_mesh->get_model_matrix());

    m_mesh->draw();
}
