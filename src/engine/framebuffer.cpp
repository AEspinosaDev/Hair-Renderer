#include "framebuffer.h"

GLIB_NAMESPACE_BEGIN

void Framebuffer::generate()
{

    GL_CHECK(glGenFramebuffers(1, &m_id));
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, m_id));

    for (Attachment &attachment : m_attachments)
    {
        if (!attachment.isRenderbuffer) // If its texture attachment...
        {
            Texture *texture = attachment.texture;

            if (!texture)
                ASSERT("No instance of texture in framebuffer attachment! Either previously create the instance or turn true -isRenderbuffer- option");

            if (!texture->is_generated())
            {
                TextureConfig newConfig = texture->get_config();
                newConfig.samples = m_samples;
                texture->set_config(newConfig);
                texture->set_extent(m_extent);
                texture->generate();
            }
            else
            {
                if (texture->get_extent() != m_extent)
                    ASSERT("Texture extent does not match framebuffer extent!");
                if (texture->get_config().samples != m_samples)
                    ASSERT("Texture sample count does not match framebuffer sample count!");
            }

            TextureType type = texture->get_config().type;
            switch (type)
            {
            case TEXTURE_2D:
                GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, attachment.attachmentType,
                                                type, texture->get_id(), texture->get_config().level));
                break;
            case TEXTURE_CUBEMAP:
                GL_CHECK(glFramebufferTexture(GL_FRAMEBUFFER, attachment.attachmentType,
                                              texture->get_id(), texture->get_config().level));
                break;
            case TEXTURE_3D || TEXTURE_2D_ARRAY:
                GL_CHECK(glFramebufferTexture3D(GL_FRAMEBUFFER, attachment.attachmentType,
                                                type, texture->get_id(), texture->get_config().level, texture->get_config().layers));
                break;
            }
        }
        else
        { // If its renderbuffer attachment...

            Renderbuffer *renderbuffer = attachment.renderbuffer;

            if (!renderbuffer)
                ASSERT("No instance of renderbuffer in framebuffer attachment! Either previously create the instance or turn false -isRenderbuffer- option");

            if (!renderbuffer->is_generated())
                renderbuffer->generate();

            renderbuffer->bind();

            if (m_samples == 1)
            {
                GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, renderbuffer->get_internal_format(), m_extent.width, m_extent.height));
            }
            else
                GL_CHECK(glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_samples, renderbuffer->get_internal_format(), m_extent.width, m_extent.height));

            GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment.attachmentType, GL_RENDERBUFFER, renderbuffer->get_id()));

            // renderbuffer->unbind();
        }
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        ERR_LOG("ERROR::FRAMEBUFFER::" << m_id << ":: Framebuffer is not complete!");

    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));

    m_generated = true;
}
void Framebuffer::bind() const
{
}
void Framebuffer::unbind() const
{
}
void Renderbuffer::generate()
{
    GL_CHECK(glGenRenderbuffers(1, &m_id));
}

void Renderbuffer::bind() const
{
    GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, m_id));
}

void Renderbuffer::unbind() const
{
    GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, 0));
}

GLIB_NAMESPACE_END
