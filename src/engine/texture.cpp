#include "texture.h"

GLIB_NAMESPACE_BEGIN

void Texture::generate()
{
    GL_CHECK(glGenTextures(1, &m_id));

    setup();

    m_generated = true;
}
void Texture::setup() const
{
    GL_CHECK(glBindTexture(m_config.type, m_id));

    switch (m_config.type)
    {
    case TEXTURE_2D:
        if (m_config.samples == 1)
        {
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
        }
        else
        {
            GL_CHECK(glTexImage2DMultisample(m_config.type,
                                             m_config.samples,
                                             m_config.internalFormat,
                                             m_extent.width,
                                             m_extent.height,
                                             GL_TRUE));
        }
        break;

    case TEXTURE_3D || TEXTURE_2D_ARRAY:
        if (m_config.samples == 1)
        {
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
        }
        else
        {
            GL_CHECK(glTexImage3DMultisample(m_config.type,
                                             m_config.samples,
                                             m_config.internalFormat,
                                             m_extent.width,
                                             m_extent.height,
                                             m_config.layers,
                                             GL_TRUE));
        }
        break;
    case TEXTURE_CUBEMAP:
        // TBD...
        break;
    }

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

GLIB_NAMESPACE_END