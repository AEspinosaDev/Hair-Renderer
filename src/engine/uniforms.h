#ifndef __UNIFORMS__
#define __UNIFORMS__

#include "core.h"
#include <vector>

GLIB_NAMESPACE_BEGIN

class UniformBuffer
{

    unsigned int m_id;
    const size_t BYTES;

    struct Layout
    {
        size_t bytes;
        size_t offset;
    };

    std::vector<Layout>
        m_layouts;

    bool m_generated{false};

public:
    UniformBuffer(const size_t sizeInBytes) : BYTES(sizeInBytes), m_layouts({{sizeInBytes, 0}}){};

    ~UniformBuffer()
    {
        GL_CHECK(glDeleteBuffers(1, &m_id))
    }

    void generate();

    void bind() const;

    void unbind() const;

    void cache_data(const size_t sizeInBytes, const void *data, const size_t offset = 0) const;

    inline bool is_generated() const { return m_generated; }
};

GLIB_NAMESPACE_END

#endif