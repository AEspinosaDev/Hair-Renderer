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
        int faceWidth = m_extent.width / 4;   // Each face is 1/4 of the total width
        int faceHeight = m_extent.height / 4; // Each face is 1/3 of the total height

        // Upload each face to the cubemap texture
        for (int i = 0; i < 6; i++)
        {
            int xOffset = i * faceWidth;
            int yOffset = 1 * faceHeight; 
            
            GL_CHECK(glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                m_config.level,
                m_config.internalFormat, //At least GL_RGB16F
                512,
                512,
                m_config.border,
                m_config.format, //GL_RGB
                m_config.dataType, //GL_FLOAT
                m_image.HDRdata));
        }
        break;
    }

    if (m_config.freeImageCacheOnGenerate)
    {
        if (m_image.data)
            free(m_image.data);
        if (m_image.HDRdata)
            free(m_image.HDRdata);
    }

    if (m_config.type != TEXTURE_2D_MULTISAMPLE && m_config.type != TEXTURE_2D_MULRISAMPLE_ARRAY)
    {
        if (m_config.useMipmaps)
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