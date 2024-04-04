#include "texture.h"

GLIB_NAMESPACE_BEGIN

void Texture::generate()
{
    GL_CHECK(glGenTextures(1, &m_id));

    GL_CHECK(glBindTexture(m_config.type, m_id));

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

    if (m_config.useMipmaps)
    {
        GL_CHECK(glGenerateMipmap(m_config.type));
        GL_CHECK(glTexParameterf(m_config.type, GL_TEXTURE_MIN_FILTER, m_config.minFilter));
        GL_CHECK(glTexParameterf(m_config.type, GL_TEXTURE_MAG_FILTER, m_config.magFilter));
    }

    GL_CHECK(glTexParameterf(m_config.type, GL_TEXTURE_WRAP_T, m_config.wrapT));
    GL_CHECK(glTexParameterf(m_config.type, GL_TEXTURE_WRAP_S, m_config.wrapS));

    if (m_config.anisotropicFilter)
    {

        float fLargest;
        GL_CHECK(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest));
        GL_CHECK(glTexParameterf(m_config.type, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest));
    }

    GL_CHECK(glBindTexture(m_config.type, 0));

    m_generated = true;
}

inline void Texture::set_extent(Extent2D extent)
{
    if (m_generated)
    {
        resize(extent);
    }
    else
        m_extent = extent;
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

    GL_CHECK(glBindTexture(m_config.type, m_id));

    GL_CHECK(glTexImage2D(m_config.type, m_config.level, m_config.internalFormat, m_extent.width, m_extent.height, m_config.border, m_config.format,
                          m_config.type, m_image.data));

    GL_CHECK(glBindTexture(m_config.type, 0));

    // }
    // else {
    // 	GLcall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_RendererID));
    // 	GLcall(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_Samples, m_TextConfig.internalFormat, m_Width, m_Height,
    // 		GL_TRUE));
    // }
}

GLIB_NAMESPACE_END