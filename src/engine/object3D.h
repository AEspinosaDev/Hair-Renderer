#ifndef __OBJECT_3D__
#define __OBJECT_3D__

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Transform
{

    glm::mat4 worldMatrix;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    glm::vec3 right;
    glm::vec3 up;
    glm::vec3 forward;

public:
    Transform(
        glm::mat4 worldMatrix = glm::mat4(1.0f),
        glm::vec3 rotation = glm::vec3(0.0f),
        glm::vec3 scale = glm::vec3(1.0f),
        glm::vec3 position = glm::vec3(0.0f),
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3 forward = glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f)

            ) : position(position),
                scale(scale),
                rotation(rotation),
                up(up),
                forward(forward),
                right(right),
                worldMatrix(worldMatrix)
    {
    }
};

typedef enum Object3DType
{
    MESH,
    CAMERA,
    LIGHT,
} Object3DType;

class Object3D
{
protected:
    const char *m_name;

    Transform m_transform;

    std::vector<Object3D *> m_children;
    Object3D *m_parent;

    const Object3DType TYPE;

    bool m_enabled{true};
    bool m_isDirty{true};

public:
    Object3D(const char *na, glm::vec3 p, Object3DType t) : TYPE(t), m_name(na),
                                                            m_parent(nullptr)
    {
        m_transform.position = p;
    }

    Object3D(glm::vec3 p, Object3DType t) : TYPE(t), m_name(""),
                                            m_parent(nullptr)
    {
        m_transform.position = p;
    }
    Object3D(Object3DType t) : TYPE(t), m_name(""),
                               m_parent(nullptr)
    {
        m_transform.position = glm::vec3(0.0f);
    }

    ~Object3D()
    {
        // delete[] children;
        delete m_parent;
    }

    virtual inline Object3DType get_type() const { return TYPE; };
    virtual void set_position(const glm::vec3 p)
    {
        m_transform.position = p;
        m_isDirty = true;
    }

    virtual inline glm::vec3 get_position() const { return m_transform.position; };

    virtual void set_rotation(const glm::vec3 p)
    {
        m_transform.rotation = p;

        // Update forward
        glm::vec3 direction;
        direction.x = cos(glm::radians(p.x)) * cos(glm::radians(p.y));
        direction.y = sin(glm::radians(p.y));
        direction.z = sin(glm::radians(p.x)) * cos(glm::radians(p.y));
        m_transform.forward = -glm::normalize(direction);
        // Update up

        // Update right
        m_transform.right = glm::cross(m_transform.forward, m_transform.up);

        m_isDirty = true;
    }

    virtual inline glm::vec3 get_rotation() const { return m_transform.rotation; };

    virtual void set_scale(const glm::vec3 s)
    {
        m_transform.scale = s;
        m_isDirty = true;
    }

    virtual void set_scale(const float s)
    {
        m_transform.scale = glm::vec3(s);
        m_isDirty = true;
    }

    virtual inline glm::vec3 get_scale() const { return m_transform.scale; }

    virtual inline Transform get_transform() const { return m_transform; }

    virtual inline void set_active(const bool s)
    {
        m_enabled = s;
        m_isDirty = true;
    }

    virtual inline float get_pitch() const { return m_transform.rotation.y; }

    virtual inline void set_pitch(float p) { set_rotation({m_transform.rotation.x, p, m_transform.rotation.z}); }

    virtual inline void set_yaw(float p) { set_rotation({p, m_transform.rotation.y, m_transform.rotation.z}); }

    virtual inline float get_yaw() const { return m_transform.rotation.x; }

    virtual inline bool is_active() { return m_enabled; }

    virtual inline bool is_dirty() { return m_isDirty; }

    virtual inline const char *get_name() const { return m_name; }

    virtual inline void set_name(const char *s) { m_name = s; }

    virtual void set_transform(Transform t)
    {
        m_transform = t;
        m_isDirty = true;
    }

    virtual glm::mat4 get_model_matrix()
    {
        if (m_isDirty)
        {

            m_transform.worldMatrix = glm::mat4(1.0f);
            m_transform.worldMatrix = glm::translate(m_transform.worldMatrix, m_transform.position);
            m_transform.worldMatrix = glm::rotate(m_transform.worldMatrix, m_transform.rotation.x, glm::vec3(1, 0, 0));
            m_transform.worldMatrix = glm::rotate(m_transform.worldMatrix, m_transform.rotation.y, glm::vec3(0, 1, 0));
            m_transform.worldMatrix = glm::rotate(m_transform.worldMatrix, m_transform.rotation.z, glm::vec3(0, 0, 1));
            m_transform.worldMatrix = glm::scale(m_transform.worldMatrix, m_transform.scale);

            //  Dirty flag
            m_transform.worldMatrix = m_parent ? m_parent->get_model_matrix() * m_transform.worldMatrix : m_transform.worldMatrix;

            m_isDirty = false;
        }
        return m_transform.worldMatrix;
    }

    virtual void add_child(Object3D *child)
    {
        child->m_parent = this;
        m_children.push_back(child);
    }

    virtual std::vector<Object3D *> get_children() const { return m_children; }

    virtual Object3D *get_parent() const { return m_parent; }
};

#endif