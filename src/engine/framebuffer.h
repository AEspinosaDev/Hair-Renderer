#ifndef __FRAMEBUFFER__
#define __FRAMEBUFFER__

#include <vector>
#include "texture.h"

GLIB_NAMESPACE_BEGIN

class Renderbuffer; // Definition on the bottom

struct Attachment
{
    // Necessary data for texture based attachment
    Texture *texture{nullptr};
    // Necessary data for renderbuffer based attachment
    Renderbuffer *renderbuffer{nullptr};
    // For color attachments
    unsigned int attachmentType{GL_COLOR_ATTACHMENT0};
    // Is a renderbuffer? Texture is unused
    bool isRenderbuffer{false};
};

class Framebuffer
{

    unsigned int m_id;

    Extent2D m_extent;

    unsigned int m_samples;

    std::vector<Attachment> m_attachments;

    bool m_resizable{true};

    bool m_generated{false};

public:
    Framebuffer(Extent2D extent, std::vector<Attachment> attachments, unsigned int samples = 1) : m_extent(extent), m_attachments(attachments), m_samples(samples) {}
    ~Framebuffer()
    {
        GL_CHECK(glDeleteFramebuffers(1, &m_id));
        // delete m_attachments;
    };
    void generate();

    inline bool is_generated() const { return m_generated; }

    void bind() const;

    void unbind() const;
};

class Renderbuffer
{
    unsigned int m_id;
    unsigned int m_interalFormat;
    bool m_generated{false};

public:
    Renderbuffer(unsigned int internatFormat = GL_DEPTH24_STENCIL8)
        : m_interalFormat(m_interalFormat){};

    ~Renderbuffer()
    {
        GL_CHECK(glDeleteRenderbuffers(1, &m_id));
    };

    void generate();

    inline unsigned int get_id() const { return m_id; }

    inline unsigned int get_internal_format() const { return m_interalFormat; }

    inline bool is_generated() const { return m_generated; }

    void bind() const;

    void unbind() const;
};

GLIB_NAMESPACE_END

#endif
