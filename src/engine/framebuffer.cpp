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
            {
                ERR_LOG("ERROR::FRAMEBUFFER::No instance of texture in framebuffer attachment! Either previously create the instance or turn true -isRenderbuffer- option");
                ASSERT(texture);
            }

            if (!texture->is_generated())
            {
                // Update just in case texture key config
                TextureConfig newConfig = texture->get_config();
                newConfig.samples = m_samples;
                texture->set_config(newConfig);
                texture->set_extent(m_extent);
                texture->generate();
            }
            else
            {
                if (texture->get_extent() != m_extent)
                    ERR_LOG("ERROR::FRAMEBUFFER::Texture extent does not match framebuffer extent!");
                if (texture->get_config().samples != m_samples)
                    ERR_LOG("ERROR::FRAMEBUFFER::Texture sample count does not match framebuffer sample count!");
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

            if (!renderbuffer){
                ERR_LOG("No instance of renderbuffer in framebuffer attachment! Either previously create the instance or turn false -isRenderbuffer- option");
                ASSERT(renderbuffer);
            }

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
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, m_id));
}
void Framebuffer::unbind() const
{
    Framebuffer::bind_default();
}

void Framebuffer::bind_default()
{
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
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

void Framebuffer::set_extent(Extent2D extent)
{
    resize(extent);
}

void Framebuffer::resize(Extent2D extent)
{
    m_extent = extent;

    if (m_generated)
    {
        for (Attachment &attachment : m_attachments)
        {
            if (!attachment.isRenderbuffer && attachment.texture)
            {
                attachment.texture->resize(extent);
            }
            else if (attachment.renderbuffer && attachment.renderbuffer)
            {
                attachment.renderbuffer->bind();
                if (m_samples == 1)
                {
                    GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, attachment.renderbuffer->get_internal_format(), m_extent.width, m_extent.height));
                }
                else
                    GL_CHECK(glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_samples, attachment.renderbuffer->get_internal_format(), m_extent.width, m_extent.height));

                GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment.attachmentType, GL_RENDERBUFFER, attachment.renderbuffer->get_id()));

                attachment.renderbuffer->unbind();
            }
        }
    }
}

void Framebuffer::blit(const Framebuffer *const dst, unsigned int mask, unsigned int filter,
                       Extent2D srcExtent, Extent2D dstExtent,
                       Position2D srcOrigin, Position2D dstOrigin) const
{
    GL_CHECK(glBindFramebuffer(GL_READ_FRAMEBUFFER, m_id));
    GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst ? dst->get_id() : 0));

    GL_CHECK(glBlitFramebuffer(srcOrigin.x, srcOrigin.y, srcExtent.width, srcExtent.height,
                               dstOrigin.x, dstOrigin.y, dstExtent.width, dstExtent.height,
                               mask, filter));
}

void Framebuffer::clear_color_bit()
{
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));
}

void Framebuffer::clear_color_depth_bit()
{
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void Framebuffer::clear_depth_bit()
{
    GL_CHECK(glClear(GL_DEPTH_BUFFER_BIT));
}

void Framebuffer::clear_bits(unsigned int bits)
{
    GL_CHECK(glClear(bits));
}

void Framebuffer::enable_depth_test(bool op)
{
    op ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
}

void Framebuffer::enable_depth_writes(bool op)
{
    GL_CHECK(glDepthMask(op));
}

GLIB_NAMESPACE_END
