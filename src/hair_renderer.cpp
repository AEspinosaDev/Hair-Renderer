#include "hair_renderer.h"

#define YUKSEL

void HairRenderer::init()
{
#pragma region INIT
    Renderer::init();

    chdir("/home/tony/Dev/Hair-Renderer/");

    m_camera = new Camera(m_window.extent.width, m_window.extent.height, {0.0f, 0.0f, -10.0f});

    m_controller = new Controller(m_camera);

    m_vignette = Mesh::create_screen_quad();

    m_hair = new Mesh();
    m_head = new Mesh();

    m_floor = new Mesh();
    loaders::load_OBJ(m_floor, "resources/models/plane.obj");
    m_floor->set_scale(50.0f);
    m_floor->set_position({0.0f, -4.0f, 0.0f});

    m_light.light = new PointLight();
    m_light.dummy = new Mesh();
    loaders::load_OBJ(m_light.dummy, "resources/models/sphere.obj");
    m_light.set_position({6.0f, 3.0f, -6.0f});

#pragma endregion
#pragma region FRAMEBUFFERS

    // Creating multisampled forward pass buffer

    TextureConfig masaaColorConfig{};
    masaaColorConfig.type = TextureType::TEXTURE_2D_MULTISAMPLE;
    masaaColorConfig.format = GL_RGBA;
    masaaColorConfig.internalFormat = GL_RGBA16;
    masaaColorConfig.dataType = GL_UNSIGNED_BYTE;

    Attachment msaaColorAttachment{};
    msaaColorAttachment.texture = new Texture(m_window.extent, masaaColorConfig);
    msaaColorAttachment.attachmentType = GL_COLOR_ATTACHMENT0;
    Attachment msaaDepthAttachment{};
    msaaDepthAttachment.isRenderbuffer = true;
    msaaDepthAttachment.renderbuffer = new Renderbuffer(GL_DEPTH24_STENCIL8);
    msaaDepthAttachment.attachmentType = GL_DEPTH_STENCIL_ATTACHMENT;

    m_multisampledFBO = new Framebuffer(m_window.extent, {msaaColorAttachment, msaaDepthAttachment}, m_globalSettings.samples);
    m_multisampledFBO->generate();

    // Creating the shadow pass buffer

    TextureConfig depthConfig{};
    depthConfig.format = GL_DEPTH_COMPONENT;
    depthConfig.internalFormat = GL_DEPTH_COMPONENT24;
    depthConfig.dataType = GL_FLOAT;
    depthConfig.anisotropicFilter = false;
    depthConfig.magFilter = GL_NEAREST;
    depthConfig.minFilter = GL_NEAREST;
    depthConfig.wrapS = GL_CLAMP_TO_BORDER;
    depthConfig.wrapT = GL_CLAMP_TO_BORDER;
    depthConfig.useMipmaps = false;
    depthConfig.borderColor = glm::vec4(1.0f);

    Attachment depthAttachment{};
    depthAttachment.texture = new Texture(m_globalSettings.shadowExtent, depthConfig);
    depthAttachment.attachmentType = GL_DEPTH_ATTACHMENT;

    m_shadowFBO = new Framebuffer(m_globalSettings.shadowExtent, {depthAttachment});
    m_shadowFBO->generate();

#pragma endregion
#pragma region SHADER PIPELINES

    // Uniform buffers
    m_cameraUBO = new UniformBuffer(sizeof(CameraUniforms), UBOLayout::CAMERA_LAYOUT);
    m_cameraUBO->generate();

    m_globalUBO = new UniformBuffer(sizeof(GlobalUniforms), UBOLayout::GLOBAL_LAYOUT);
    m_globalUBO->generate();

    GraphicPipeline litPipeline{};
    litPipeline.shader = new Shader("resources/shaders/cook-torrance.glsl", ShaderType::LIT);
    litPipeline.shader->set_uniform_block("Camera", UBOLayout::CAMERA_LAYOUT);
    litPipeline.shader->set_uniform_block("Scene", UBOLayout::GLOBAL_LAYOUT);
    Material *headMaterial = new Material(litPipeline);
    headMaterial->set_texture("u_shadowMap", m_shadowFBO->get_attachments().front().texture);
    m_head->set_material(headMaterial);
    Material *floorMaterial = new Material(litPipeline);
    floorMaterial->set_texture("u_shadowMap", m_shadowFBO->get_attachments().front().texture);
    m_floor->set_material(floorMaterial);

    GraphicPipeline hairPipeline{};
#ifdef MARSCHNER
    hairPipeline.shader = new Shader("resources/shaders/strand-marschner.glsl", ShaderType::LIT);
#else
    hairPipeline.shader = new Shader("resources/shaders/strand-kajiya.glsl", ShaderType::LIT);
#endif
    hairPipeline.shader->set_uniform_block("Camera", UBOLayout::CAMERA_LAYOUT);
    hairPipeline.shader->set_uniform_block("Scene", UBOLayout::GLOBAL_LAYOUT);
    Material *hairMaterial = new Material(hairPipeline);
    hairMaterial->set_texture("u_shadowMap", m_shadowFBO->get_attachments().front().texture);
    m_hair->set_material(hairMaterial);

    GraphicPipeline unlitPipeline{};
    unlitPipeline.shader = new Shader("resources/shaders/unlit.glsl", ShaderType::UNLIT);
    unlitPipeline.shader->set_uniform_block("Camera", UBOLayout::CAMERA_LAYOUT);
    Material *lightMaterial = new Material(unlitPipeline);
    m_light.dummy->set_material(lightMaterial);

    m_depthPipeline.shader = new Shader("resources/shaders/depth.glsl", ShaderType::UNLIT);

    GraphicPipeline screenPipeline{};
    screenPipeline.shader = new Shader("resources/shaders/screen.glsl", ShaderType::UNLIT);
    Material *screenMaterial = new Material(screenPipeline);
    screenMaterial->set_texture("u_frame", m_multisampledFBO->get_attachments().front().texture);
    m_vignette->set_material(screenMaterial);

#pragma endregion

#pragma region MESH LOADING

#ifdef YUKSEL
    // CEM YUKSEL MODELS
    {
        std::thread loadThread1(loaders::load_PLY, m_head, "resources/models/woman.ply", true, true, false);
        loadThread1.detach();
        m_head->set_rotation({180.0f, -90.0f, 0.0f});
        m_head->set_scale(0.98f);
        std::thread loadThread2(hair_loaders::load_cy_hair, m_hair, "resources/models/natural.hair");
        loadThread2.detach();
        m_hair->set_scale(0.054f);
        m_hair->set_position({0.015f, -0.2f, 0.3f});
        m_hair->set_rotation({-90.0f, 0.0f, 16.7f});
    }
#else
    // NEURAL HAIRCUT MODELS
    {
        loaders::load_PLY(m_head, "resources/models/head_blender.ply", true, true, false);
        std::thread loadThread1(hair_loaders::load_neural_hair, m_hair, "resources/models/2000000.ply", m_head, true, true, false);
        loadThread1.detach();
    }
#endif
#pragma endregion
}

void HairRenderer::update()
{
    if (!user_interface_wants_to_handle_input())
        m_controller->handle_keyboard(m_window.ptr, 0, 0, m_time.delta);
}

void HairRenderer::draw()
{

    // Setup UBOs
    CameraUniforms camu;
    camu.vp = m_camera->get_projection() * m_camera->get_view();
    camu.mv = m_camera->get_view() * m_head->get_model_matrix();
    camu.v = m_camera->get_view();
    m_cameraUBO->cache_data(sizeof(CameraUniforms), &camu);

    GlobalUniforms globu;
    globu.ambient = {m_globalSettings.ambientColor,
                     m_globalSettings.ambientStrength};
    glm::vec3 lightViewSpace = camu.v * glm::vec4(m_light.light->get_position(), 1.0f); // Transform to view space
    globu.lightPos = {lightViewSpace, 1.0f};
    globu.lightColor = {m_light.light->get_color(), m_light.light->get_intensity()};
    ShadowConfig shadow = m_light.light->get_shadow_config();
    glm::mat4 lp = glm::perspective(glm::radians(shadow.fov),
                                    1.0f,
                                    shadow.nearPlane,
                                    shadow.farPlane);
    glm::mat4 lv = glm::lookAt(m_light.light->get_position(), shadow.target, glm::vec3(0, 1, 0));
    globu.shadowConfig = {shadow.bias, shadow.pcfKernel, shadow.cast, 0.0f};
    globu.lightViewProj = lp * lv;
    m_globalUBO->cache_data(sizeof(GlobalUniforms), &globu);

    if (m_light.light->get_cast_shadows())
        shadow_pass();

    forward_pass();

    screen_pass();
}

#pragma region FORWARD PASS
void HairRenderer::forward_pass()
{
    m_multisampledFBO->bind();
    Framebuffer::clear_color_depth_bit();

    resize_viewport(m_window.extent);

    // ----- Draw ----

    MaterialUniforms headu;
    headu.mat4Types["u_model"] = m_head->get_model_matrix();
    headu.vec3Types["u_albedo"] = m_headSettings.skinColor;
    m_head->get_material()->set_uniforms(headu);

    m_head->draw();

    MaterialUniforms hairu;
#ifdef MARSCHNER
    hairu.vec3Types["u_hair.baseColor"] = m_hairSettings.baseColor;
    hairu.floatTypes["u_hair.specular"] = m_hairSettings.specular;
    hairu.floatTypes["u_hair.roughness"] = m_hairSettings.roughness;
    hairu.floatTypes["u_hair.scatter"] = m_hairSettings.scatter;
    hairu.floatTypes["u_hair.shift"] = m_hairSettings.shift;
    hairu.floatTypes["u_hair.ior"] = m_hairSettings.ior;
    hairu.boolTypes["u_hair.r"] = m_hairSettings.r;
    hairu.boolTypes["u_hair.tt"] = m_hairSettings.tt;
    hairu.boolTypes["u_hair.trt"] = m_hairSettings.trt;
#else
    hairu.vec3Types["u_albedo"] = m_hairSettings.color;
    hairu.vec3Types["u_spec1"] = m_hairSettings.specColor1;
    hairu.floatTypes["u_specPwr1"] = m_hairSettings.specPower1;
    hairu.vec3Types["u_spec2"] = m_hairSettings.specColor2;
    hairu.floatTypes["u_specPwr2"] = m_hairSettings.specPower2;
#endif
    hairu.floatTypes["u_thickness"] = m_hairSettings.thickness;
    hairu.mat4Types["u_model"] = m_hair->get_model_matrix();
    hairu.vec3Types["u_camPos"] = m_camera->get_position();
    m_hair->get_material()->set_uniforms(hairu);

    m_hair->draw(true, GL_LINES);

    MaterialUniforms dummyu;
    m_light.dummy->set_position(m_light.light->get_position());
    dummyu.mat4Types["u_model"] = m_light.dummy->get_model_matrix();
    dummyu.boolTypes["u_useVertexColor"] = false;
    dummyu.vec3Types["u_baseColor"] = glm::vec3(1.0f);
    m_light.dummy->get_material()->set_uniforms(dummyu);

    m_light.dummy->draw();

    MaterialUniforms flooru;
    flooru.mat4Types["u_model"] = m_floor->get_model_matrix();
    flooru.vec3Types["u_albedo"] = glm::vec3(1.0);
    m_floor->get_material()->set_uniforms(flooru);

    m_floor->draw();
}
#pragma endregion

#pragma region SHADOW PASS
void HairRenderer::shadow_pass()
{

    m_shadowFBO->bind();
    Framebuffer::clear_depth_bit();
    Framebuffer::enable_depth_writes(true);
    Framebuffer::enable_depth_test(true);

    resize_viewport(m_globalSettings.shadowExtent);

    m_depthPipeline.shader->bind();

    m_depthPipeline.shader->set_mat4("u_model", m_head->get_model_matrix());
    m_head->draw(false);

    m_depthPipeline.shader->set_mat4("u_model", m_floor->get_model_matrix());
    m_floor->draw(false);

    m_depthPipeline.shader->set_mat4("u_model", m_hair->get_model_matrix());

    m_hair->draw(false, GL_LINES);

    m_depthPipeline.shader->unbind();
}
#pragma endregion
#pragma region SCREEN PASS
void HairRenderer::screen_pass()
{
    Framebuffer::blit(m_multisampledFBO, nullptr, GL_COLOR_BUFFER_BIT, GL_NEAREST, m_window.extent, m_window.extent);

    // Framebuffer::bind_default();

    // m_vignette->draw();
}
#pragma endregion

void HairRenderer::setup_user_interface_frame()
{
    Renderer::setup_user_interface_frame();

    ImGui::NewFrame();

    // ImGui::ShowDemoWindow(&m_globalSettings.showUI);
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(420, 920), ImGuiCond_Once);
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
#ifdef MARSCHNER
    ImGui::ColorEdit3("Base color", (float *)&m_hairSettings.baseColor);
    ImGui::DragFloat("Specular", &m_hairSettings.specular, .05f, 0.0f, 8.0f);
    ImGui::DragFloat("Roughness", &m_hairSettings.roughness, .05f, 0.0f, 1.0f);
    ImGui::DragFloat("Scatter", &m_hairSettings.scatter, .1f, 0.0f, 5.0f);
    ImGui::DragFloat("Shift", &m_hairSettings.shift, -0.05f, 2 * M_PI, M_PI_2);
    ImGui::DragFloat("IOR", &m_hairSettings.ior, 0.01f, 0.0f, 10.0f);
    ImGui::Checkbox("R Lobe", &m_hairSettings.r);
    ImGui::Checkbox("TT Lobe", &m_hairSettings.tt);
    ImGui::Checkbox("TRT Lobe", &m_hairSettings.trt);
#else
    ImGui::ColorEdit3("Base color", (float *)&m_hairSettings.color);
    ImGui::DragFloat("Specular 1 power", &m_hairSettings.specPower1, 1.0f, 0.0f, 240.0f);
    ImGui::DragFloat("Specular 2 power", &m_hairSettings.specPower2, 1.0f, 0.0f, 240.0f);
#endif
    ImGui::Separator();
    ImGui::SeparatorText("Head Settings");
    gui::draw_transform_widget(m_head);
    ImGui::ColorEdit3("Skin color", (float *)&m_headSettings.skinColor);
    ImGui::Separator();
    ImGui::SeparatorText("Lighting Settings");
    ImGui::ColorEdit3("Ambient color", (float *)&m_globalSettings.ambientColor);
    ImGui::DragFloat("Ambient intensity", &m_globalSettings.ambientStrength, 0.1f, 0.0f, 10.0f);
    gui::draw_light_widget(m_light.light);
    gui::draw_transform_widget(m_light.light);

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
