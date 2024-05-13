#include "texture.h"

GLIB_NAMESPACE_BEGIN

Shader *Texture::HDRIConverterShader = nullptr;

void Texture::generate()
{
    GL_CHECK(glGenTextures(1, &m_id));

    setup();

    m_generated = true;
}
void Texture::setup()
{
    GL_CHECK(glBindTexture(m_config.type, m_id));

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
            m_image.data));
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
            m_image.data));
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
                nullptr));
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

    //Free temporal resources used
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