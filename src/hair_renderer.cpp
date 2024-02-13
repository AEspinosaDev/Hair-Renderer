#include "hair_renderer.h"

void HairRenderer::init()
{

    Renderer::init();

    chdir("/home/tony/Dev/OpenGL-Hair/");

    m_camera = new Camera(m_window.extent.width, m_window.extent.height,{0.0f,0.0f,-10.0f});

    m_controller = new Controller(m_camera);

    m_hairShader = new Shader("resources/shaders/test.glsl", ShaderType::LIT);
    m_headShader = new Shader("resources/shaders/cook-torrance.glsl", ShaderType::LIT);

    m_light = new PointLight();
    m_light->set_position({5.0f, 3.0f, -7.0f});

    m_hair = new Mesh();
    m_head = new Mesh();

    // CEM YUKSEL MODELS
    {
        std::thread loadThread1(loaders::load_PLY, m_head, "resources/models/woman.ply", true, true, false);
        loadThread1.detach();
        m_head->set_rotation({180.0f, -90.0f, 0.0f});
        std::thread loadThread2(loaders::load_cy_hair, m_hair, "resources/models/natural.hair");
        loadThread2.detach();
        m_hair->set_scale(0.054f);
        m_hair->set_position({0.015f, -0.09f, 0.2f});
        m_hair->set_rotation({-90.0f, 0.0f, 16.7f});
    }

    // NEURAL HAIRCUT MODELS
    {
        // std::thread loadThread1(loaders::load_neural_hair, m_hair, "resources/models/hair_blender.ply", true, true, false);
        // loadThread1.detach();
        // std::thread loadThread2(loaders::load_PLY, m_head, "resources/models/head_blender.ply", true, true, false);
        // loadThread2.detach();
    }

    glDisable(GL_CULL_FACE);
}

void HairRenderer::update()
{
    if (!user_interface_wants_to_handle_input())
        m_controller->handle_keyboard(m_window.ptr, 0, 0, m_time.delta);
}

void HairRenderer::draw()
{
    clearColorDepthBit();

    m_headShader->bind();

    m_headShader->set_mat4("u_viewProj", m_camera->get_projection() * m_camera->get_view());
    m_headShader->set_mat4("u_modelView", m_camera->get_view() * m_head->get_model_matrix());
    m_headShader->set_mat4("u_model", m_head->get_model_matrix());
    m_headShader->set_mat4("u_view", m_camera->get_view());
    m_light->cache_uniforms(m_headShader);
    m_headShader->set_vec3("u_skinColor", m_headSettings.skinColor);

    m_head->draw();

    m_hairShader->bind();
    m_hairShader->set_float("u_thickness", m_hairSettings.thickness);

    m_hairShader->set_mat4("u_viewProj", m_camera->get_projection() * m_camera->get_view());
    m_hairShader->set_mat4("u_modelView", m_camera->get_view() * m_hair->get_model_matrix());
    m_hairShader->set_mat4("u_model", m_hair->get_model_matrix());
    m_hairShader->set_mat4("u_view", m_camera->get_view());
    m_light->cache_uniforms(m_hairShader);

    glLineWidth(m_hairSettings.thickness);
    m_hair->draw(GL_LINES);
}

void HairRenderer::setup_user_interface_frame()
{
    Renderer::setup_user_interface_frame();

    ImGui::NewFrame();

    // ImGui::ShowDemoWindow(&m_globalSettings.showUI);

    ImGui::Begin("Settings", &m_globalSettings.showUI); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
    ImGui::SeparatorText("Profiler");
    ImGui::Text(" %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Separator();
    ImGui::SeparatorText("Global Settings");
    if (ImGui::Checkbox("V-Sync", &m_settings.vSync))
    {
        set_v_sync(m_settings.vSync);
    }
    ImGui::Separator();
    ImGui::SeparatorText("Hair Settings");
    gui::draw_transform_widget(m_hair);
    ImGui::DragFloat("Strand thickness", &m_hairSettings.thickness, 0.001f, 0.001f, 0.05f);
    ImGui::Separator();
    ImGui::SeparatorText("Head Settings");
    gui::draw_transform_widget(m_head);
    ImGui::ColorEdit3("Skin color", (float *)&m_headSettings.skinColor);
    ImGui::Separator();
    ImGui::SeparatorText("Lighting Settings");
    gui::draw_transform_widget(m_light);
    ImGui::Separator();

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
