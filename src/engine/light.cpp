#include "light.h"

int PointLight::INSTANCED_POINT_LIGHTS = 0;
int DirectionalLight::INSTANCED_DIRECTIONAL_LIGHTS = 0;

void PointLight::cache_uniforms(Shader *shader) const
{
    shader->set_vec3("u_lightPos",m_transform.position);
}
void DirectionalLight::cache_uniforms(Shader *shader) const
{
}