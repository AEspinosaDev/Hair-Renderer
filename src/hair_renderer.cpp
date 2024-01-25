#include "hair_renderer.h"

void HairRenderer::init()
{

    Renderer::init();

    chdir("/home/tony/Dev/OpenGL-Hair/");

    m_camera = new Camera(m_window.extent.width, m_window.extent.height);

    m_controller = new Controller(m_camera);

    // m_shader = new Shader("resources/shaders/phong.glsl",  ShaderType::LIT);
    m_shader = new Shader("resources/shaders/test.glsl", ShaderType::UNLIT);

    m_mesh = new Mesh();
    Geometry g;
    g.vertices = {{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                  {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                  {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
                  {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}}};

    g.indices = {0, 1, 2, 2, 3, 0};

    Geometry strands;
    strands.vertices = {{{-0.5f, -0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                        {{0.0f, -0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                        {{0.5f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
                        {{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}};
    strands.indices = {0,1,2,3};

    // m_mesh->set_geometry(g);
    m_mesh->set_geometry(strands);

    // loaders::load_OBJ(m_mesh, false, "resources/models/cube.obj");
    // loaders::load_NeuralHair(m_mesh, false, "resources/models/00100000_strands_points.ply", true, true);

    m_mesh->generate_buffers();

    glDisable(GL_CULL_FACE);
}

void HairRenderer::update()
{
    std::cout << user_interface_wants_to_handle_input() << std::endl;
    if (!user_interface_wants_to_handle_input())
        m_controller->handle_keyboard(m_window.ptr, 0, 0, m_time.delta);
}

void HairRenderer::draw()
{
    clearColorDepthBit();

    m_shader->bind();

    m_shader->set_mat4("u_viewProj", m_camera->get_projection() * m_camera->get_view());
    m_shader->set_mat4("u_modelView", m_camera->get_view() * m_mesh->get_model_matrix());
    m_shader->set_mat4("u_model", m_mesh->get_model_matrix());
    m_shader->set_mat4("u_view", m_camera->get_view());

    glPointSize(3.0);
    glLineWidth(2.0);
    m_mesh->draw(GL_LINES);
}

void HairRenderer::setup_user_interface_frame()
{
    Renderer::setup_user_interface_frame();

    ImGui::NewFrame();

    ImGui::ShowDemoWindow(&m_showUI);

    ImGui::Begin("Profiler", &m_showUI); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
    ImGui::Text(" %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();

    ImGui::Render();
}

void HairRenderer::setup_window_callbacks()
{
    glfwSetWindowUserPointer(m_window.ptr, this);

    glfwSetKeyCallback(m_window.ptr, [](GLFWwindow *w, int key, int scancode, int action, int mods)
                       { static_cast<HairRenderer *>(glfwGetWindowUserPointer(w))->key_callback(w, key, scancode, action, mods); });
    glfwSetCursorPosCallback(m_window.ptr, [](GLFWwindow *w, double x, double y)
                             { static_cast<HairRenderer *>(glfwGetWindowUserPointer(w))->mouse_callback(w, x, y); });
    glfwSetFramebufferSizeCallback(m_window.ptr, [](GLFWwindow *w, int width, int height)
                                   { static_cast<HairRenderer *>(glfwGetWindowUserPointer(w))->resize_callback(w, width, height); });
}
