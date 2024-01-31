#include "light.h"

int PointLight::INSTANCED_POINT_LIGHTS = 0;
int DirectionalLight::INSTANCED_DIRECTIONAL_LIGHTS = 0;

void PointLight::cache_uniforms(Shader *shader) const
{
    shader->set_vec3("u_lightPos", m_transform.position);
}
void DirectionalLight::cache_uniforms(Shader *shader) const
{
}

void Light::user_interface_frame(bool displayName, bool includeParent)
{
    if (displayName)
        ImGui::Text(m_name);

    if (includeParent)
        Object3D::user_interface_frame();

    ImGui::Spacing();
    ImGui::SeparatorText("Light");

    float intensity = get_intensity();
    if (ImGui::DragFloat("Intensity", &intensity, 0.005f, 0.0f, 10.0f))
        set_intensity(intensity);
    glm::vec3 color = get_color();
    if (ImGui::ColorEdit3("Color", (float *)&color))
    {
        set_color(color);
    };

    bool castShadows = get_cast_shadows();

    if (ImGui::Checkbox("Cast Shadows", &castShadows))
    {
        set_cast_shadows(castShadows);
    };

    ImGui::Spacing();
    ImGui::Separator();
}

void PointLight::user_interface_frame(bool displayName, bool includeParent)
{
    Light::user_interface_frame(displayName,includeParent);
}

void DirectionalLight::user_interface_frame(bool displayName, bool includeParent)
{
    Light::user_interface_frame(displayName,includeParent);
}
