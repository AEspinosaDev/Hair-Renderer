#include "uniforms.h"

GLIB_NAMESPACE_BEGIN

void UniformBuffer::generate()
{
    GL_CHECK(glGenBuffers(1, &m_id));
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, m_id));
    GL_CHECK(glBufferData(GL_UNIFORM_BUFFER, BYTES, NULL, GL_STATIC_DRAW));
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0));

    size_t i = 0;
    for (Layout &layout : m_layouts)
    {
        GL_CHECK(glBindBufferRange(GL_UNIFORM_BUFFER, m_layoutBinding + i, m_id, layout.offset, layout.bytes));
        i++;
    }
    m_generated = true;
}

void UniformBuffer::cache_data(const size_t sizeInBytes, const void *data, const size_t offset) const
{
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, m_id));
    glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeInBytes, data);
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}

void UniformBuffer::bind() const
{
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, m_id));
}

void UniformBuffer::unbind() const
{
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}

GLIB_NAMESPACE_END
