#include "hair_renderer.h"

//----------------------------------------------
// Hair model type
#define YUKSEL

//----------------------------------------------
// Antialiasing implementation defines
// #define FXAA
#define SMAA
#define SMAAx2

//----------------------------------------------
// Shading algorithms setup
#define EPIC

//----------------------------------------------
// Misc defines
#define GLINT_EXTENT 16
#define DEPTH_PREPASS
// #define TEST

void HairRenderer::init()
{
#pragma region INIT
    Renderer::init();

    chdir("/home/tony/Dev/Hair-Renderer/");

    m_camera = new Camera(m_window.extent.width, m_window.extent.height, {0.0f, 0.0f, -10.0f});

    m_controller = new Controller(m_camera);

    m_vignette = Mesh::create_screen_quad();
    m_skybox = Mesh::create_cube();

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

    // Noise text generation pass
    TextureConfig noiseConfig{};
    noiseConfig.format = GL_RED;
    noiseConfig.internalFormat = GL_R8;
    noiseConfig.dataType = GL_UNSIGNED_BYTE;
    noiseConfig.anisotropicFilter = false;
    noiseConfig.magFilter = GL_LINEAR;
    noiseConfig.minFilter = GL_LINEAR;
    noiseConfig.wrapS = GL_REPEAT;
    noiseConfig.wrapT = GL_REPEAT;
    noiseConfig.useMipmaps = true;

    Attachment noiseAttachment{};
    noiseAttachment.texture = new Texture({GLINT_EXTENT, GLINT_EXTENT}, noiseConfig);
    noiseAttachment.attachmentType = GL_COLOR_ATTACHMENT0;

    m_noiseFBO = new Framebuffer({GLINT_EXTENT, GLINT_EXTENT}, {noiseAttachment});
    m_noiseFBO->generate();

    // Creating multisampled forward pass buffer

#if defined(SMAA) || defined(FXAA)
    TextureConfig colorConfig{};
#ifdef SMAAx2
    colorConfig.type = TextureType::TEXTURE_2D_MULTISAMPLE;
#else
    colorConfig.type = TextureType::TEXTURE_2D;
#endif
    colorConfig.format = GL_RGBA;
    colorConfig.internalFormat = GL_RGBA16;
    colorConfig.dataType = GL_UNSIGNED_BYTE;
    colorConfig.useMipmaps = false;

    Attachment colorAttachment{};
    colorAttachment.texture = new Texture(m_window.extent, colorConfig);
    colorAttachment.attachmentType = GL_COLOR_ATTACHMENT0;
    Attachment depthAttachment{};
    depthAttachment.isRenderbuffer = true;
    depthAttachment.renderbuffer = new Renderbuffer(GL_DEPTH24_STENCIL8);
    depthAttachment.attachmentType = GL_DEPTH_STENCIL_ATTACHMENT;

#ifdef SMAAx2
    m_forwardFBO = new Framebuffer(m_window.extent, {colorAttachment, depthAttachment}, 2);
#else
    m_forwardFBO = new Framebuffer(m_window.extent, {colorAttachment, depthAttachment});
#endif
    m_forwardFBO->generate();

#else
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

    m_forwardFBO = new Framebuffer(m_window.extent, {msaaColorAttachment, msaaDepthAttachment}, m_globalSettings.samples);
    m_forwardFBO->generate();
#endif

#ifdef SMAA
#ifdef SMAAx2
    TextureConfig separateConfig{};
    separateConfig.format = GL_RGBA;
    separateConfig.internalFormat = GL_RGBA16;
    separateConfig.dataType = GL_UNSIGNED_BYTE;
    separateConfig.useMipmaps = false;
    separateConfig.wrapR = GL_CLAMP_TO_EDGE;
    separateConfig.wrapS = GL_CLAMP_TO_EDGE;
    separateConfig.wrapT = GL_CLAMP_TO_EDGE;

    Attachment separateAttachment{};
    separateAttachment.texture = new Texture(m_window.extent, separateConfig);
    separateAttachment.attachmentType = GL_COLOR_ATTACHMENT0;

    m_smaaRes.separateFBO = new Framebuffer(m_window.extent, {separateAttachment});
    m_smaaRes.separateFBO->generate();

#endif

    TextureConfig edgeConfig{};
    edgeConfig.format = GL_RGBA;
    edgeConfig.internalFormat = GL_RGBA16;
    edgeConfig.dataType = GL_UNSIGNED_BYTE;
    edgeConfig.useMipmaps = false;
    edgeConfig.wrapR = GL_CLAMP_TO_EDGE;
    edgeConfig.wrapS = GL_CLAMP_TO_EDGE;
    edgeConfig.wrapT = GL_CLAMP_TO_EDGE;

    Attachment edgeAttachment{};
    edgeAttachment.texture = new Texture(m_window.extent, edgeConfig);
    edgeAttachment.attachmentType = GL_COLOR_ATTACHMENT0;
#ifdef SMAAx2
    Attachment edgeAttachment1{};
    edgeAttachment1.texture = new Texture(m_window.extent, edgeConfig);
    edgeAttachment1.attachmentType = GL_COLOR_ATTACHMENT1;
#endif

    Attachment edgeDepthAttachment{};
    edgeDepthAttachment.isRenderbuffer = true;
    edgeDepthAttachment.renderbuffer = new Renderbuffer(GL_DEPTH24_STENCIL8);
    edgeDepthAttachment.attachmentType = GL_DEPTH_STENCIL_ATTACHMENT;

#ifdef SMAAx2
    m_smaaRes.edgeFBO = new Framebuffer(m_window.extent, {edgeAttachment, edgeAttachment1, edgeDepthAttachment});
#else
    m_smaaRes.edgeFBO = new Framebuffer(m_window.extent, {edgeAttachment, edgeDepthAttachment});

#endif
    m_smaaRes.edgeFBO->generate();

    TextureConfig blendConfig{};
    blendConfig.format = GL_RGBA;
    blendConfig.internalFormat = GL_RGBA16;
    blendConfig.dataType = GL_UNSIGNED_BYTE;
    blendConfig.useMipmaps = false;
    blendConfig.wrapR = GL_CLAMP_TO_EDGE;
    blendConfig.wrapS = GL_CLAMP_TO_EDGE;
    blendConfig.wrapT = GL_CLAMP_TO_EDGE;

    Attachment blendAttachment{};
    blendAttachment.texture = new Texture(m_window.extent, blendConfig);
    blendAttachment.attachmentType = GL_COLOR_ATTACHMENT0;
#ifdef SMAAx2
    Attachment blendAttachment1{};
    blendAttachment1.texture = new Texture(m_window.extent, blendConfig);
    blendAttachment1.attachmentType = GL_COLOR_ATTACHMENT1;
#endif

    Attachment blendDepthAttachment{};
    blendDepthAttachment.isRenderbuffer = true;
    blendDepthAttachment.renderbuffer = new Renderbuffer(GL_DEPTH24_STENCIL8);
    blendDepthAttachment.attachmentType = GL_DEPTH_STENCIL_ATTACHMENT;

#ifdef SMAAx2
    m_smaaRes.blendFBO = new Framebuffer(m_window.extent, {blendAttachment, blendAttachment1, blendDepthAttachment});
#else
    m_smaaRes.blendFBO = new Framebuffer(m_window.extent, {blendAttachment, blendDepthAttachment});
#endif
    m_smaaRes.blendFBO->generate();

    // AUXILIAR TEXS

    TextureConfig searchConfig{};
    searchConfig.format = GL_RED;
    searchConfig.internalFormat = GL_R8;
    searchConfig.wrapR = GL_CLAMP_TO_BORDER;
    searchConfig.wrapS = GL_CLAMP_TO_BORDER;
    searchConfig.wrapT = GL_CLAMP_TO_BORDER;
    searchConfig.freeImageCacheOnGenerate = false;

    m_smaaRes.searchTex = new Texture({SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT}, searchConfig);
    Image searchImg{};
    searchImg.data = searchTexBytes;
    searchImg.channels = 1;
    searchImg.extent = {SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT};
    m_smaaRes.searchTex->set_image(searchImg);
    // loaders::load_image(m_smaaRes.searchTex, "resources/images/SearchTexOGL.png");

    TextureConfig areaConfig{};
    areaConfig.format = GL_RG;
    areaConfig.internalFormat = GL_RG8;
    areaConfig.wrapR = GL_CLAMP_TO_BORDER;
    areaConfig.wrapS = GL_CLAMP_TO_BORDER;
    areaConfig.wrapT = GL_CLAMP_TO_BORDER;
    areaConfig.freeImageCacheOnGenerate = false;

    m_smaaRes.areaTex = new Texture({AREATEX_WIDTH, AREATEX_HEIGHT}, areaConfig);
    Image areaImg{};
    areaImg.data = areaTexBytes;
    areaImg.channels = 2;
    areaImg.extent = {AREATEX_WIDTH, AREATEX_HEIGHT};
    m_smaaRes.areaTex->set_image(areaImg);
    // loaders::load_image(m_smaaRes.areaTex, "resources/images/AreaTexOGL.png");

    m_smaaRes.searchTex->generate();
    m_smaaRes.areaTex->generate();

#endif

    TextureConfig depthConfig{};
    depthConfig.format = GL_DEPTH_COMPONENT;
    depthConfig.internalFormat = GL_DEPTH_COMPONENT32;
    depthConfig.dataType = GL_FLOAT;
    depthConfig.anisotropicFilter = false;
    depthConfig.magFilter = GL_NEAREST;
    depthConfig.minFilter = GL_NEAREST;
    depthConfig.wrapS = GL_CLAMP_TO_BORDER;
    depthConfig.wrapT = GL_CLAMP_TO_BORDER;
    depthConfig.useMipmaps = false;
    depthConfig.borderColor = glm::vec4(1.0f);

    Attachment shadowDepthAttachment{};
    shadowDepthAttachment.texture = new Texture(m_globalSettings.shadowExtent, depthConfig);
    shadowDepthAttachment.attachmentType = GL_DEPTH_ATTACHMENT;

    m_shadowFBO = new Framebuffer(m_globalSettings.shadowExtent, {shadowDepthAttachment});
    m_shadowFBO->generate();

    Attachment predepthAttachment{};
    predepthAttachment.texture = new Texture(m_window.extent, depthConfig);
    predepthAttachment.attachmentType = GL_DEPTH_ATTACHMENT;

    m_depthFBO = new Framebuffer(m_window.extent, {predepthAttachment});
    m_depthFBO->generate();

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

    GraphicPipeline hairPipeline{};
#ifdef MARSCHNER
#ifdef EPIC
    hairPipeline.shader = new Shader("resources/shaders/strand-marschner-epic.glsl", ShaderType::LIT);
#else
#ifdef TEST
    hairPipeline.shader = new Shader("resources/shaders/strand-marschner-pre-test.glsl", ShaderType::LIT);
#else
    hairPipeline.shader = new Shader("resources/shaders/strand-marschner-pre.glsl", ShaderType::LIT);
#endif
#endif
#else
    hairPipeline.shader = new Shader("resources/shaders/strand-kajiya.glsl", ShaderType::LIT);
#endif
    hairPipeline.shader->set_uniform_block("Camera", UBOLayout::CAMERA_LAYOUT);
    hairPipeline.shader->set_uniform_block("Scene", UBOLayout::GLOBAL_LAYOUT);

    GraphicPipeline unlitPipeline{};
    unlitPipeline.shader = new Shader("resources/shaders/unlit.glsl", ShaderType::UNLIT);
    unlitPipeline.shader->set_uniform_block("Camera", UBOLayout::CAMERA_LAYOUT);
    Material *lightMaterial = new Material(unlitPipeline);
    m_light.dummy->set_material(lightMaterial);

    m_shadowPipeline.shader = new Shader("resources/shaders/shadow.glsl", ShaderType::OTHER);

    m_noisePipeline.shader = new Shader("resources/shaders/noise-gen.glsl", ShaderType::OTHER);

#ifdef DEPTH_PREPASS
    m_strandDepthPipeline.shader = new Shader("resources/shaders/strand-depth.glsl", ShaderType::OTHER);
    m_depthPipeline.shader = new Shader("resources/shaders/depth.glsl", ShaderType::OTHER);
#endif

#ifdef FXAA
    m_fxaaPipeline.shader = new Shader("resources/shaders/fxaa.glsl", ShaderType::OTHER);
    m_fxaaPipeline.shader->bind();
    m_fxaaPipeline.shader->set_int("u_frame", 0);
    m_fxaaPipeline.shader->unbind();
#endif

#ifdef SMAA
    m_smaaRes.edgePipeline.shader = new Shader("resources/shaders/smaa/edge-detection.glsl", ShaderType::OTHER);
    m_smaaRes.blendPipeline.shader = new Shader("resources/shaders/smaa/blending-weight.glsl", ShaderType::OTHER);
    m_smaaRes.resolvePipeline.shader = new Shader("resources/shaders/smaa/neighbour-blending.glsl", ShaderType::OTHER);
#endif

    GraphicPipeline skyboxPipeline{};
    skyboxPipeline.shader = new Shader("resources/shaders/skybox.glsl", ShaderType::OTHER);
    skyboxPipeline.state.depthFunction = DepthFuncType::LEQUAL;

#pragma endregion
#pragma region MATERIALS

    Material *headMaterial = new Material(litPipeline);
    headMaterial->set_texture("u_shadowMap", m_shadowFBO->get_attachments().front().texture);
    Texture *skin = new Texture();
    loaders::load_image(skin, "resources/images/head.png");
    skin->generate();
    headMaterial->set_texture("u_albedoMap", skin, 1);
    // headMaterial->set_texture("u_depthMap", m_depthFBO->get_attachments().front().texture, 2);
    m_head->set_material(headMaterial);

    Material *floorMaterial = new Material(litPipeline);
    floorMaterial->set_texture("u_shadowMap", m_shadowFBO->get_attachments().front().texture);
    m_floor->set_material(floorMaterial);

    Material *hairMaterial = new Material(hairPipeline);
    hairMaterial->set_texture("u_shadowMap", m_shadowFBO->get_attachments().front().texture);
    hairMaterial->set_texture("u_noiseMap", m_noiseFBO->get_attachments().front().texture, 1);
    hairMaterial->set_texture("u_depthMap", m_depthFBO->get_attachments().front().texture, 2);
    m_hair->set_material(hairMaterial);

    Material *skyboxMaterial = new Material(skyboxPipeline);

    TextureConfig skymapConfig{};
    skymapConfig.type = TextureType::TEXTURE_CUBEMAP;
    skymapConfig.format = GL_RGB;
    skymapConfig.internalFormat = GL_RGB32F;
    skymapConfig.dataType = GL_FLOAT;
    skymapConfig.anisotropicFilter = false;
    skymapConfig.wrapS = GL_CLAMP_TO_EDGE;
    skymapConfig.wrapT = GL_CLAMP_TO_EDGE;
    skymapConfig.wrapR = GL_CLAMP_TO_EDGE;

    Texture *skymap = new Texture({2048, 2048}, skymapConfig);
    loaders::load_image(skymap, "resources/images/room.hdr", true);
    skymap->generate();
    skyboxMaterial->set_texture("u_skymap", skymap);

    m_skybox->set_material(skyboxMaterial);

    Texture *irradianceTexture = skymap->compute_irradiance(32);
    headMaterial->set_texture("u_irradianceMap", irradianceTexture, 2);
    hairMaterial->set_texture("u_irradianceMap", irradianceTexture, 3);

#ifndef EPIC
    TextureConfig lutConfig{};
    // lutConfig.format = GL_RGB;
    // lutConfig.internalFormat = GL_RGB8;
    // lutConfig.anisotropicFilter = true;
    lutConfig.useMipmaps = false;
    lutConfig.wrapR = GL_CLAMP_TO_BORDER;
    lutConfig.wrapS = GL_CLAMP_TO_BORDER;
    lutConfig.wrapT = GL_CLAMP_TO_BORDER;

    // Marschner M term
    Texture *marschnerM = new Texture(lutConfig);
    loaders::load_image(marschnerM, "resources/images/m.png");
    marschnerM->generate();
    hairMaterial->set_texture("u_m", marschnerM, 4);
    // Marschner N term
    Texture *marschnerN = new Texture(lutConfig);
    loaders::load_image(marschnerN, "resources/images/sqn.png");
    marschnerN->generate();
    hairMaterial->set_texture("u_n", marschnerN, 5);
#endif

#pragma endregion

#pragma region MESH LOADING

#ifdef YUKSEL
    // CEM YUKSEL MODELS
    {
        std::thread loadThread1(loaders::load_PLY, m_head, "resources/models/woman.ply", true, true, false);
        loadThread1.detach();
        m_head->set_rotation({180.0f, -90.0f, 0.0f});
        m_head->set_scale(0.98f);
        std::thread loadThread2(hair_loaders::load_cy_hair, m_hair, "resources/models/straight.hair");
        loadThread2.detach();

        // Low poly
        // m_hair->set_scale(0.054f);
        // m_hair->set_position({0.015f, -0.2f, 0.3f});
        // m_hair->set_rotation({-90.0f, 0.0f, 16.7f});

        m_hair->set_scale(0.048f);
        m_hair->set_position({0.015f, 0.3f, 0.1f});
        m_hair->set_rotation({-90.0f, 0.0f, 90.7f});
    }
#else
    // NEURAL HAIRCUT MODELS
    {
        loaders::load_PLY(m_head, "resources/models/head_blender.ply", true, true, false);
        std::thread loadThread1(hair_loaders::load_neural_hair, m_hair, "resources/models/2000000.ply", m_head, true, true, false);
        loadThread1.detach();
        m_head->set_scale(3.0);
        m_hair->set_scale(3.0);
    }
#endif

#ifdef TEST

    m_hair = Mesh::create_strand();
    m_hair->generate_buffers();
    m_hair->set_material(hairMaterial);
#endif

    // Generate noise texture
    noise_pass();
    m_noiseFBO->get_attachments().front().texture->generate_mipmaps();

#pragma endregion
}

void HairRenderer::update()
{
    if (!user_interface_wants_to_handle_input())
        m_controller->handle_keyboard(m_window.ptr, 0, 0, m_time.delta);

    if (m_light.animated)
    {
        float rotationAngle = glm::radians(10.0f * m_time.delta);
        float _x = m_light.light->get_position().x * cos(rotationAngle) - m_light.light->get_position().z * sin(rotationAngle);
        float _z = m_light.light->get_position().x * sin(rotationAngle) + m_light.light->get_position().z * cos(rotationAngle);

        m_light.light->set_position({_x, m_light.light->get_position().y, _z});
        m_light.dummy->set_position(m_light.light->get_position());
    }
}

void HairRenderer::draw()
{

    // Setup UBOs
    CameraUniforms camu;
    camu.vp = m_camera->get_projection() * m_camera->get_view();
    camu.mv = m_camera->get_view() * m_head->get_model_matrix();
    camu.v = m_camera->get_view();
    camu.position = m_camera->get_position();
    camu.exposure = m_globalSettings.exposure;
    m_cameraUBO->cache_data(sizeof(CameraUniforms), &camu);

    GlobalUniforms globu;
    globu.ambient = {m_globalSettings.ambientColor,
                     m_globalSettings.ambientStrength};
    glm::vec3 lightViewSpace = camu.v * glm::vec4(m_light.light->get_position(), 1.0f); // Transform to view space
    // glm::vec3 lightViewSpace = m_light.light->get_position();
    globu.lightPos = {lightViewSpace, 1.0f};
    globu.lightColor = {m_light.light->get_color(), m_light.light->get_intensity()};
    ShadowConfig shadow = m_light.light->get_shadow_config();
    glm::mat4 lp = glm::perspective(glm::radians(shadow.fov),
                                    1.0f,
                                    shadow.nearPlane,
                                    shadow.farPlane);
    glm::mat4 lv = glm::lookAt(m_light.light->get_position(), shadow.target, glm::vec3(0, 1, 0));
    globu.shadowConfig = {shadow.bias, shadow.pcfKernel, shadow.cast, shadow.kernelRadius};
    globu.lightViewProj = lp * lv;
    globu.frustrumData = {m_camera->get_near(), m_camera->get_far(),
                          shadow.nearPlane, shadow.farPlane};
    m_globalUBO->cache_data(sizeof(GlobalUniforms), &globu);

    if (m_light.light->get_cast_shadows())
        shadow_pass();

#ifdef DEPTH_PREPASS
    depth_prepass();
#endif

    forward_pass();

    postprocess_pass();
}

#pragma region FORWARD PASS
void HairRenderer::forward_pass()
{

    m_forwardFBO->bind();

    Framebuffer::clear_color_depth_bit();

    resize_viewport(m_window.extent);

    // ----- Draw ----

    MaterialUniforms headu;
    headu.mat4Types["u_model"] = m_head->get_model_matrix();
    headu.vec3Types["u_albedo"] = m_headSettings.skinColor;
    headu.boolTypes["u_hasAlbedoTex"] = m_headSettings.useAlbedoTexture;
    headu.boolTypes["u_useSkybox"] = m_globalSettings.useSkyboxIrradiance;
    m_head->get_material()->set_uniforms(headu);

#ifndef TEST
    m_head->draw();
#endif

    MaterialUniforms hairu;
#ifdef MARSCHNER
    hairu.vec3Types["u_hair.baseColor"] = m_hairSettings.baseColor;
    hairu.floatTypes["u_hair.Rpower"] = m_hairSettings.Rpower;
    hairu.floatTypes["u_hair.TTpower"] = m_hairSettings.TTpower;
    hairu.floatTypes["u_hair.TRTpower"] = m_hairSettings.TRTpower;
    hairu.floatTypes["u_hair.roughness"] = m_hairSettings.roughness;
    hairu.floatTypes["u_hair.scatter"] = m_hairSettings.scatterExp;
    hairu.floatTypes["u_hair.shift"] = m_hairSettings.shift;
    hairu.floatTypes["u_hair.ior"] = m_hairSettings.ior;
    hairu.boolTypes["u_hair.r"] = m_hairSettings.r;
    hairu.boolTypes["u_hair.tt"] = m_hairSettings.tt;
    hairu.boolTypes["u_hair.trt"] = m_hairSettings.trt;
    hairu.boolTypes["u_hair.glints"] = m_hairSettings.glints;
    hairu.boolTypes["u_hair.useScatter"] = m_hairSettings.scatter;
    hairu.boolTypes["u_hair.coloredScatter"] = m_hairSettings.colorScatter;
    hairu.boolTypes["u_hair.occlusion"] = m_hairSettings.occlusion;
    hairu.boolTypes["u_useSkybox"] = m_globalSettings.useSkyboxIrradiance;

    glm::vec3 bvcenter = m_hair->get_bounding_volume() ? static_cast<Sphere *>(m_hair->get_bounding_volume())->center : glm::vec3(0.0);
    hairu.vec3Types["u_BVCenter"] = glm::vec3(m_hair->get_model_matrix() * glm::vec4(bvcenter.x,
                                                                                     bvcenter.y,
                                                                                     bvcenter.z,
                                                                                     1.0));
#else
    hairu.vec3Types["u_albedo"] = m_hairSettings.color;
    hairu.vec3Types["u_spec1"] = m_hairSettings.specColor1;
    hairu.floatTypes["u_specPwr1"] = m_hairSettings.specPower1;
    hairu.vec3Types["u_spec2"] = m_hairSettings.specColor2;
    hairu.floatTypes["u_specPwr2"] = m_hairSettings.specPower2;
#endif
    hairu.floatTypes["u_thickness"] = m_hairSettings.thickness;
    hairu.mat4Types["u_model"] = m_hair->get_model_matrix();
    // hairu.vec3Types["u_camPos"] = m_camera->get_position();
    m_hair->get_material()->set_uniforms(hairu);

#ifdef TEST
    m_hair->draw(true);
#else
    m_hair->draw(true, GL_LINES);
#endif

    MaterialUniforms dummyu;
    m_light.dummy->set_position(m_light.light->get_position());
    dummyu.mat4Types["u_model"] = m_light.dummy->get_model_matrix();
    dummyu.boolTypes["u_useVertexColor"] = false;
    dummyu.vec3Types["u_baseColor"] = glm::vec3(1.0f);
    m_light.dummy->get_material()->set_uniforms(dummyu);

    m_light.dummy->draw();

    // MaterialUniforms flooru;
    // flooru.mat4Types["u_model"] = m_floor->get_model_matrix();
    // flooru.vec3Types["u_albedo"] = glm::vec3(1.0);
    // m_floor->get_material()->set_uniforms(flooru);
    // m_floor->draw();

    m_skybox->set_rotation({0.0, m_globalSettings.enviromentRotation, 0.0});
    MaterialUniforms skyu;
    skyu.mat4Types["u_viewProj"] = m_camera->get_projection() * glm::mat4(glm::mat3(m_camera->get_view())); // Take out the transform
    skyu.mat4Types["u_model"] = m_skybox->get_model_matrix();
    m_skybox->get_material()->set_uniforms(skyu);

    m_skybox->draw();
}
#pragma endregion
#pragma region DEPTH PRE PASS
void HairRenderer::depth_prepass()
{
    m_depthFBO->bind();
    Framebuffer::clear_color_depth_bit();
    Framebuffer::enable_depth_writes(true);
    Framebuffer::enable_depth_test(true);

    resize_viewport(m_window.extent);

    m_depthPipeline.shader->bind();
    m_depthPipeline.shader->set_mat4("u_model", m_head->get_model_matrix());
    m_head->draw(false);
    m_depthPipeline.shader->unbind();

    m_strandDepthPipeline.shader->bind();
    m_strandDepthPipeline.shader->set_mat4("u_model", m_hair->get_model_matrix());
    m_strandDepthPipeline.shader->set_float("u_thickness", m_hairSettings.thickness);
    m_strandDepthPipeline.shader->set_vec3("u_camPos", m_camera->get_position());
    m_hair->draw(false, GL_LINES);
    m_strandDepthPipeline.shader->unbind();
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

    m_shadowPipeline.shader->bind();

    m_shadowPipeline.shader->set_mat4("u_model", m_head->get_model_matrix());
    m_shadowPipeline.shader->set_bool("u_isHair", false);
    m_head->draw(false);

    // m_depthPipeline.shader->set_mat4("u_model", m_floor->get_model_matrix());
    // m_floor->draw(false);

    m_shadowPipeline.shader->set_mat4("u_model", m_hair->get_model_matrix());
    m_shadowPipeline.shader->set_bool("u_isHair", true);

    m_hair->draw(false, GL_LINES);

    m_shadowPipeline.shader->unbind();
}
#pragma endregion
#pragma region NOISE PASS
void HairRenderer::noise_pass()
{
    m_noiseFBO->bind();

    resize_viewport({GLINT_EXTENT, GLINT_EXTENT});

    m_noisePipeline.shader->bind();

    m_vignette->draw(false);

    m_noisePipeline.shader->unbind();
}

#pragma endregion
#pragma region POST PROCESS PASS
void HairRenderer::postprocess_pass()
{
#ifndef FXAA
#ifdef SMAA
    smaa_pass();
#else
    Framebuffer::blit(m_forwardFBO, nullptr, GL_COLOR_BUFFER_BIT, GL_NEAREST, m_window.extent, m_window.extent);
#endif
#else

    Framebuffer::bind_default();

    m_fxaaPipeline.shader->bind();

    m_forwardFBO->get_attachments().front().texture->bind();

    m_vignette->draw(false);

    m_fxaaPipeline.shader->unbind();
#endif
}

void HairRenderer::smaa_pass()
{
    // 1º Edge Detection pass

    m_smaaRes.edgeFBO->bind();
    Framebuffer::clear_color_depth_bit();
    set_clear_color(glm::vec4(0.0f));

    m_smaaRes.edgePipeline.shader->bind();
    m_smaaRes.edgePipeline.shader->set_vec2("u_screen", glm::vec2(m_window.extent.width, m_window.extent.height));
    m_forwardFBO->get_attachments().front().texture->bind();
    m_vignette->draw(false);
    m_smaaRes.edgePipeline.shader->unbind();

    // 2º Blending Weight pass
    m_smaaRes.blendFBO->bind();
    Framebuffer::clear_color_depth_bit();
    set_clear_color(glm::vec4(0.0f));

    m_smaaRes.blendPipeline.shader->bind();
    m_smaaRes.blendPipeline.shader->set_vec2("u_screen", glm::vec2(m_window.extent.width, m_window.extent.height));
    m_smaaRes.edgeFBO->get_attachments().front().texture->bind();
    m_smaaRes.areaTex->bind(1);
    m_smaaRes.searchTex->bind(2);
    m_smaaRes.blendPipeline.shader->set_int("u_edgeTex", 0);
    m_smaaRes.blendPipeline.shader->set_int("u_areaTex", 1);
    m_smaaRes.blendPipeline.shader->set_int("u_searchTex", 2);
    m_vignette->draw(false);
    m_smaaRes.blendPipeline.shader->unbind();

    // 3º Neighbour Blending pass
    Framebuffer::bind_default();
    Framebuffer::clear_color_depth_bit();
    set_clear_color(glm::vec4(0.0f));

    m_smaaRes.resolvePipeline.shader->bind();
    m_smaaRes.resolvePipeline.shader->set_vec2("u_screen", glm::vec2(m_window.extent.width, m_window.extent.height));
    m_forwardFBO->get_attachments().front().texture->bind();
    m_smaaRes.blendFBO->get_attachments().front().texture->bind(1);
    m_smaaRes.resolvePipeline.shader->set_int("u_colorTex", 0);
    m_smaaRes.resolvePipeline.shader->set_int("u_blendTex", 1);
    m_vignette->draw(false);
    m_smaaRes.resolvePipeline.shader->unbind();
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
    ImGui::DragFloat("Camera Exposure", &m_globalSettings.exposure);
    ImGui::Separator();
    ImGui::SeparatorText("Hair Settings");
    gui::draw_transform_widget(m_hair);
    ImGui::DragFloat("Strand thickness", &m_hairSettings.thickness, 0.001f, 0.001f, 0.05f);
#ifdef MARSCHNER
    ImGui::ColorEdit3("Base color", (float *)&m_hairSettings.baseColor);
    ImGui::DragFloat("R Scale", &m_hairSettings.Rpower, .05f, 0.0f, 30.0f);
    ImGui::DragFloat("TT Scale", &m_hairSettings.TTpower, .05f, 0.0f, 30.0f);
    ImGui::DragFloat("TRT Scale", &m_hairSettings.TRTpower, .05f, 0.0f, 30.0f);
    ImGui::DragFloat("Roughness", &m_hairSettings.roughness, .05f, 0.0f, 1.0f);
    ImGui::DragFloat("Shift", &m_hairSettings.shift, -0.05f, 2 * M_PI, M_PI_2);
    ImGui::DragFloat("IOR", &m_hairSettings.ior, 0.01f, 0.0f, 10.0f);
    ImGui::Checkbox("Glints", &m_hairSettings.glints);
    ImGui::Separator();
    ImGui::Checkbox("Scattering", &m_hairSettings.scatter);
    ImGui::Checkbox("Color absortion", &m_hairSettings.colorScatter);
    ImGui::DragFloat("Scatter Sigma", &m_hairSettings.scatterExp, 1.f, 0.0f, 1000.0f);
    ImGui::Separator();
    ImGui::Checkbox("R Lobe", &m_hairSettings.r);
    ImGui::Checkbox("TT Lobe", &m_hairSettings.tt);
    ImGui::Checkbox("TRT Lobe", &m_hairSettings.trt);
    ImGui::Separator();
    ImGui::Checkbox("Occlusion", &m_hairSettings.occlusion);
#else
    ImGui::ColorEdit3("Base color", (float *)&m_hairSettings.color);
    ImGui::DragFloat("Specular 1 power", &m_hairSettings.specPower1, 1.0f, 0.0f, 240.0f);
    ImGui::DragFloat("Specular 2 power", &m_hairSettings.specPower2, 1.0f, 0.0f, 240.0f);
#endif
    ImGui::Separator();
    ImGui::SeparatorText("Head Settings");
    gui::draw_transform_widget(m_head);
    ImGui::ColorEdit3("Skin color", (float *)&m_headSettings.skinColor);
    ImGui::Checkbox("Use texture", &m_headSettings.useAlbedoTexture);
    ImGui::Separator();
    ImGui::SeparatorText("Lighting Settings");
    ImGui::ColorEdit3("Clear color", (float *)&m_globalSettings.ambientColor);
    ImGui::Checkbox("Use skybox as ambient light", &m_globalSettings.useSkyboxIrradiance);
    ImGui::DragFloat("Enviroment rotation", &m_globalSettings.enviromentRotation, 1.0f, -180.0f, 180.0f);
    ImGui::DragFloat("Enviroment intensity", &m_globalSettings.ambientStrength, 0.1f, 0.0f, 10.0f);
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

void HairRenderer::resize_callback(GLFWwindow *w, int width, int height)
{
    m_camera->set_projection(width, height);
    resize({width, height});
    m_forwardFBO->resize({width, height});
    m_depthFBO->resize({width, height});

#ifdef SMAA
    m_smaaRes.blendFBO->resize({width, height});
    m_smaaRes.edgeFBO->resize({width, height});
#ifdef SMAAx2
    m_smaaRes.separateFBO->resize({width, height});
#endif
#endif
}
