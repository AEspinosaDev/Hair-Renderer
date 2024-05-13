#include "material.h"

GLIB_NAMESPACE_BEGIN

void Material::bind() const
{
    m_pipeline.shader->bind();
    setup_pipeline();
    upload_uniforms();
    bind_textures();
}

void Material::unbind() const
{
    unbind_textures();
    m_pipeline.shader->unbind();
}

void Material::upload_uniforms() const
{

    for (auto &vec3u : m_uniforms.vec3Types)
    {
        m_pipeline.shader->set_vec3(vec3u.first.c_str(), vec3u.second);
    }
    for (auto &vec4u : m_uniforms.vec4Types)
    {
        m_pipeline.shader->set_vec4(vec4u.first.c_str(), vec4u.second);
    }
    for (auto &mat4u : m_uniforms.mat4Types)
    {
        m_pipeline.shader->set_mat4(mat4u.first.c_str(), mat4u.second);
    }
    for (auto &fu : m_uniforms.floatTypes)
    {
        m_pipeline.shader->set_float(fu.first.c_str(), fu.second);
    }
    for (auto &iu : m_uniforms.intTypes)
    {
        m_pipeline.shader->set_int(iu.first.c_str(), iu.second);
    }
    for (auto &bu : m_uniforms.boolTypes)
    {
        m_pipeline.shader->set_bool(bu.first.c_str(), bu.second);
    }
}

void Material::setup_pipeline() const
{
    glCullFace(m_pipeline.state.cullFace);
    m_pipeline.state.depthTest ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
    glDepthFunc(m_pipeline.state.depthFunction);
    glDepthMask(m_pipeline.state.depthWrites);
    if (m_pipeline.state.blending)
    {
        glEnable(GL_BLEND);
        glBlendFunc(m_pipeline.state.blendingFuncSRC,
                    m_pipeline.state.blendingFuncDST);
        glBlendEquation(ADD);
    }
    else
    {
        glDisable(GL_BLEND);
    }

    upload_uniforms();
    bind_textures();
}

void Material::bind_textures() const
{
    for (auto &textureData : m_textures)
    {
        textureData.second.texture->bind(textureData.second.slot);
    }
}

void Material::unbind_textures() const
{
     for (auto &textureData : m_textures)
    {
        textureData.second.texture->unbind();
    }
}

GLIB_NAMESPACE_END