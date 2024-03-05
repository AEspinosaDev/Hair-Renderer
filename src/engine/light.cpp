#include "light.h"

GLIB_NAMESPACE_BEGIN

int PointLight::INSTANCED_POINT_LIGHTS = 0;
int DirectionalLight::INSTANCED_DIRECTIONAL_LIGHTS = 0;

void PointLight::cache_uniforms(MaterialUniforms &u) const
{
    u.vec3Types["u_lightPos"] = m_transform.position;
}
void DirectionalLight::cache_uniforms(MaterialUniforms &u) const
{
}

GLIB_NAMESPACE_END