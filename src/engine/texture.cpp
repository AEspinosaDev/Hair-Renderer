#include "texture.h"

GLIB_NAMESPACE_BEGIN

Shader *Texture::HDRIConverterShader = nullptr;
Shader *Texture::IrradianceComputeShader = nullptr;

void Texture::generate()
{
    GL_CHECK(glGenTextures(1, &m_id));

    setup();

    m_generated = true;
}
void Texture::setup()
{
    GL_CHECK(glBindTexture(m_config.type, m_id));

    const void *data = nullptr;
    if (m_image.linear) // Check if image is linear
    {
        if (m_image.data)
            data = static_cast<const void *>(m_image.data);
    }
    else
    {
        if (m_image.HDRdata)
            data = static_cast<const void *>(m_image.HDRdata);
    }

    switch (m_config.type)
    {
    case TEXTURE_2D:
        GL_CHECK(glTexImage2D(
            m_config.type,
            m_config.level,
            m_config.internalFormat,
            m_extent.width,
            m_extent.height,
            m_config.border,
            m_config.format,
            m_config.dataType,
            data));
        break;

    case TEXTURE_3D || TEXTURE_2D_ARRAY:
        GL_CHECK(glTexImage3D(
            m_config.type,
            m_config.level,
            m_config.internalFormat,
            m_extent.width,
            m_extent.height,
            m_config.layers,
            m_config.border,
            m_config.format,
            m_config.dataType,
            data));
        break;
    case TEXTURE_2D_MULTISAMPLE:
        GL_CHECK(glTexImage2DMultisample(
            TEXTURE_2D_MULTISAMPLE,
            m_config.samples,
            m_config.internalFormat,
            m_extent.width,
            m_extent.height,
            GL_TRUE));
        break;
    case TEXTURE_2D_MULRISAMPLE_ARRAY:
        GL_CHECK(glTexImage3DMultisample(
            m_config.type,
            m_config.samples,
            m_config.internalFormat,
            m_extent.width,
            m_extent.height,
            m_config.layers,
            GL_TRUE));
        break;
    case TEXTURE_CUBEMAP:

        for (int i = 0; i < 6; i++)
        {
            GL_CHECK(glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                m_config.level,
                m_config.internalFormat, // At least GL_RGB16F
                m_extent.width,
                m_extent.height,
                m_config.border,
                m_config.format,   // GL_RGB
                m_config.dataType, // GL_FLOAT
                m_image.data || m_image.HDRdata ? data : nullptr));
        }
        break;
    }

    if (m_config.type != TEXTURE_2D_MULTISAMPLE && m_config.type != TEXTURE_2D_MULRISAMPLE_ARRAY)
    {
        if (m_config.useMipmaps || !m_image.panorama)
        {
            GL_CHECK(glGenerateMipmap(m_config.type));
        }
        GL_CHECK(glTexParameterf(m_config.type, GL_TEXTURE_MIN_FILTER, m_config.minFilter));
        GL_CHECK(glTexParameterf(m_config.type, GL_TEXTURE_MAG_FILTER, m_config.magFilter));

        GL_CHECK(glTexParameterf(m_config.type, GL_TEXTURE_WRAP_T, m_config.wrapT));
        GL_CHECK(glTexParameterf(m_config.type, GL_TEXTURE_WRAP_S, m_config.wrapS));
        GL_CHECK(glTexParameterf(m_config.type, GL_TEXTURE_WRAP_R, m_config.wrapR));

        GL_CHECK(glTexParameterfv(m_config.type, GL_TEXTURE_BORDER_COLOR, (float *)&m_config.borderColor));

        if (m_config.anisotropicFilter)
        {

            float fLargest;
            GL_CHECK(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest));
            GL_CHECK(glTexParameterf(m_config.type, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest));
        }
    }

    GL_CHECK(glBindTexture(m_config.type, 0));

    if (m_image.panorama)
        panorama_to_cubemap();

    if (m_config.freeImageCacheOnGenerate)
    {
        if (m_image.data)
            free(m_image.data);
        if (m_image.HDRdata)
            free(m_image.HDRdata);
    }
}

void Texture::set_extent(Extent2D extent)
{
    resize(extent);
}

void Texture::bind(unsigned int slot) const
{
    GL_CHECK(glActiveTexture(GL_TEXTURE0 + slot));
    GL_CHECK(glBindTexture(m_config.type, m_id));
}

void Texture::unbind() const
{
    GL_CHECK(glBindTexture(m_config.type, 0));
}

void Texture::resize(Extent2D extent)
{
    m_extent = extent;
    if (m_generated)
        setup();
}

void Texture::generate_mipmaps()
{
    bind();
    GL_CHECK(glGenerateMipmap(m_config.type));
}

Texture *Texture::compute_irradiance(int resolution)
{
    if (m_config.type != TextureType::TEXTURE_CUBEMAP)
    {
        ERR_LOG("Texture must be a CUBEMAP in order to compute irradiance");
        return nullptr;
    }

    if (!Texture::IrradianceComputeShader) // If null, create utility irradiance compute shader
    {
        ShaderStageSource source{};
        source.vertexBit = utils::IrradianceComputeVertexSource;
        source.fragmentBit = utils::IrradianceComputeFragmentSource;
        Texture::IrradianceComputeShader = new Shader(source, ShaderType::OTHER);
    }

    Texture *irradianceMap = new Texture({resolution, resolution}, m_config);
    irradianceMap->generate();

    // Creation of capture FBO
    unsigned int captureFBO, captureRBO;
    GL_CHECK(glGenFramebuffers(1, &captureFBO));
    GL_CHECK(glGenRenderbuffers(1, &captureRBO));
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, captureFBO));
    GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, captureRBO));
    GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, resolution, resolution));
    GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO));

    // Create projection an view matrices
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] =
        {
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))};

    GLuint cubeVBO, cubeVAO;
    GL_CHECK(glGenVertexArrays(1, &cubeVAO));
    GL_CHECK(glGenBuffers(1, &cubeVBO));
    GL_CHECK(glBindVertexArray(cubeVAO));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, cubeVBO));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(utils::cubeVertices), utils::cubeVertices, GL_STATIC_DRAW));
    GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0));
    GL_CHECK(glEnableVertexAttribArray(0));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));

    Texture::IrradianceComputeShader->bind();
    Texture::IrradianceComputeShader->set_int("u_envMap", 0);
    Texture::IrradianceComputeShader->set_mat4("u_proj", captureProjection);

    bind();

    GL_CHECK(glViewport(0, 0, resolution, resolution));
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, captureFBO));
    for (unsigned int i = 0; i < 6; ++i)
    {
        Texture::IrradianceComputeShader->set_mat4("u_view", captureViews[i]);
        GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                        GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap->get_id(), 0));
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    GL_CHECK(glBindVertexArray(0));
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));

    //Free temp resources
    GL_CHECK(glDeleteFramebuffers(1, &captureFBO));
    GL_CHECK(glDeleteRenderbuffers(1, &captureRBO));
    GL_CHECK(glDeleteVertexArrays(1, &cubeVAO));

    return irradianceMap;
}

void Texture::panorama_to_cubemap()
{
    if (!Texture::HDRIConverterShader) // If null, create utility converter shader
    {
        ShaderStageSource source{};
        source.vertexBit = utils::HDRIConverterVertexSource;
        source.fragmentBit = utils::HDRIConverterFragmentSource;
        Texture::HDRIConverterShader = new Shader(source, ShaderType::OTHER);
    }

    // Create panorama texture
    unsigned int panoramaID;
    GL_CHECK(glGenTextures(1, &panoramaID));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, panoramaID));

    GL_CHECK(glTexImage2D(
        GL_TEXTURE_2D,
        m_config.level,
        m_config.internalFormat,
        m_image.extent.width,
        m_image.extent.height,
        m_config.border,
        m_config.format,
        m_config.dataType,
        m_image.HDRdata));

    GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D));
    GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_config.minFilter));
    GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_config.magFilter));
    GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_config.wrapT));
    GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_config.wrapS));

    GL_CHECK(glBindVertexArray(1));

    unsigned int converterFBO;
    GL_CHECK(glGenFramebuffers(1, &converterFBO));

    for (int i = 0; i < 6; ++i)
    {

        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, converterFBO));
        GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER,
                                        GL_COLOR_ATTACHMENT0,
                                        GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                        m_id,
                                        0));

        bind();

        GL_CHECK(glViewport(0, 0, m_extent.width, m_extent.height));
        GL_CHECK(glClearColor(0.5f, 0.5f, 0.5f, 0.f));
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        Texture::HDRIConverterShader->bind();

        GL_CHECK(glActiveTexture(GL_TEXTURE0 + 0));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, panoramaID));

        Texture::HDRIConverterShader->set_int("u_panorama", 0);
        Texture::HDRIConverterShader->set_int("u_currentFace", i);

        GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 3));

        Texture::HDRIConverterShader->unbind();
    }

    // Free temporal resources used
    GL_CHECK(glDeleteTextures(1, &panoramaID));
    GL_CHECK(glDeleteFramebuffers(1, &converterFBO));

    bind();
    if (m_config.useMipmaps)
    {
        // Regenerate mipmaps with newly computed data
        GL_CHECK(glGenerateMipmap(m_config.type));
    }
}

GLIB_NAMESPACE_END