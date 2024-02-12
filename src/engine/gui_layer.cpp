#include "gui_layer.h"

namespace glib
{

    void gui::draw_transform_widget(Object3D *obj)
    {
        ImGui::PushID(obj);
        ImGui::Spacing();
        ImGui::SeparatorText("Transform");

        const float PI = 3.14159265359f;

        float position[3] = {obj->get_position().x,
                             obj->get_position().y,
                             obj->get_position().z};

        // std::string posTag = "Position"+std::to_string(obj->get_name());
        if (ImGui::DragFloat3("Position", position, 0.1f))
        {
            obj->set_position(glm::vec3(position[0], position[1], position[2]));
        };
        float rotation[3] = {obj->get_rotation().x,
                             obj->get_rotation().y,
                             obj->get_rotation().z};
        if (ImGui::DragFloat3("Rotation", rotation, 0.1f))
        {
            obj->set_rotation(glm::vec3(rotation[0] ,
                                        rotation[1] ,
                                        rotation[2] ));
        };
        float scale[3] = {obj->get_scale().x,
                          obj->get_scale().y,
                          obj->get_scale().z};
        if (ImGui::DragFloat3("Scale", scale, 0.1f))
        {
            obj->set_scale(glm::vec3(scale[0], scale[1], scale[2]));
        };

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::PopID();
    }

    void gui::draw_light_widget(Light *l)
    {
        ImGui::Spacing();
        ImGui::SeparatorText("Light");

        float intensity = l->get_intensity();
        if (ImGui::DragFloat("Intensity", &intensity, 0.005f, 0.0f, 10.0f))
            l->set_intensity(intensity);
        glm::vec3 color = l->get_color();
        if (ImGui::ColorEdit3("Color", (float *)&color))
        {
            l->set_color(color);
        };

        bool castShadows = l->get_cast_shadows();

        if (ImGui::Checkbox("Cast Shadows", &castShadows))
        {
            l->set_cast_shadows(castShadows);
        };

        ImGui::Spacing();
        ImGui::Separator();
    }

    void gui::draw_point_light_widget(PointLight *l)
    {
    }

    void gui::draw_directional_light_widget(PointLight *l)
    {
    }

    void gui::draw_mesh_widget(Mesh *m)
    {
    }

    void gui::draw_camera_widget(Camera *cam)
    {
        ImGui::Spacing();
        ImGui::SeparatorText("Camera");

        float _far = cam->get_far();
        float _near = cam->get_near();
        float fov = cam->get_field_of_view();

        if (ImGui::DragFloat("Near", &_near, 0.05f, 0.0f, 10.0))
            cam->set_near(_near);
        if (ImGui::DragFloat("Far", &_far, 0.1f, 0.0f, 9999.0f))
            cam->set_far(_far);
        if (ImGui::DragFloat("Field of view", &fov, 0.1f, 0.0f, 160.0f))
            cam->set_field_of_view(fov);

        ImGui::Spacing();
        ImGui::Separator();
    }
}