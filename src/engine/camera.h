#ifndef __CAMERA__
#define __CAMERA__

#include "core.h"
#include "object3D.h"

namespace glib
{

    class Camera : public Object3D
    {

    private:
        glm::mat4 m_view;
        glm::mat4 m_proj;

        float m_fov;
        float m_near;
        float m_far;
        float m_zoom;

        bool m_perspective{true};

    public:
        Camera(glm::vec3 p = glm::vec3(0.0f, 1.0f, -4.0f), glm::vec3 f = glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f)) : Object3D("Camera", p, Object3DType::CAMERA), m_fov(45.0f), m_near(.1f), m_far(100.0f) { set_rotation({-90, 0, 0}); }
        Camera(int width, int height, glm::vec3 p = glm::vec3(0.0f, 1.0f, -4.0f), glm::vec3 f = glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f)) : Object3D("Camera", p, Object3DType::CAMERA), m_fov(45.0f), m_near(.1f), m_far(100.0f)
        {
            set_rotation({-90, 0, 0});
            set_projection(width, height);
        }

        inline void set_field_of_view(float fov) { m_fov = fov; }
        inline float get_field_of_view() { return m_fov; }
        inline void set_projection(int width, int height)
        {
            if (m_perspective)
                m_proj = glm::perspective(glm::radians(m_fov), (float)width / (float)height, m_near, m_far);
        }
        inline glm::mat4 get_projection() { return m_proj; }
        inline glm::mat4 get_view() { return get_model_matrix(); }
        inline float get_far() { return m_far; }
        inline void set_far(float f) { m_far = f; }
        inline float get_near() { return m_near; }
        inline void set_near(float n) { m_near = n; }

        inline glm::mat4 get_model_matrix()
        {
            if (m_isDirty)
            {
                m_view = glm::lookAt(m_transform.position, m_transform.position + m_transform.forward, m_transform.up);
                m_isDirty = false;
            }
            return m_view;
        }
    };
}

#endif