#ifndef __MESH__
#define __MESH__

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include "object3D.h"
#include "material.h"
#include "utils.h"

GLIB_NAMESPACE_BEGIN

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec2 uv;
    glm::vec3 color;

    bool operator==(const Vertex &other) const
    {
        return position == other.position && normal == other.normal && tangent == other.tangent && uv == other.uv && color == other.color;
    }

    bool operator!=(const Vertex &other) const
    {
        return !(*this == other);
    }
};

struct Geometry
{
    size_t triangles;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    bool indexed;
};

class Mesh : public Object3D
{
protected:
    unsigned int m_vao;

    Geometry m_geometry;
    Material *m_material;

    bool m_geometry_loaded{false};
    bool m_buffer_loaded{false};

    static int INSTANCED_MESHES;

public:
    Mesh() : Object3D("Mesh", {0.0f, 0.0f, 0.0f}, Object3DType::MESH), m_material(nullptr) { Mesh::INSTANCED_MESHES++; }
    Mesh(Geometry &geometry, Material *const material) : Object3D("Mesh", {0.0f, 0.0f, 0.0f}, Object3DType::MESH), m_material(material), m_geometry(geometry) { Mesh::INSTANCED_MESHES++; }
    ~Mesh(){
        GL_CHECK(glDeleteVertexArrays(1, &m_vao));
        delete m_material;
    }

    inline unsigned int get_buffer_id() const { return m_vao; }
    inline bool is_buffer_loaded() const { return m_buffer_loaded; }

    void set_geometry(Geometry &g);

    inline Geometry get_geometry() const { return m_geometry; }

    inline void set_material(Material *const material) { m_material = material; }

    inline Material *const get_material() const { return m_material; }

    virtual void generate_buffers();

    virtual void draw(bool useMaterial = true, unsigned int drawingPrimitive = GL_TRIANGLES);

    inline static int get_number_of_instances() { return INSTANCED_MESHES; }

    /*
    Creates a screen quad for rendering textures onto the screen. Useful for postprocess and deferred screen space methods
    */
    static Mesh* create_screen_quad();

    static Mesh* create_cube();

    static Mesh* create_quad();
};


GLIB_NAMESPACE_END

namespace std
{
    template <>
    struct hash<glib::Vertex>
    {
        size_t operator()(glib::Vertex const &vertex) const
        {
            size_t seed = 0;
            glib::utils::hash_combine(seed, vertex.position, vertex.normal, vertex.tangent, vertex.uv, vertex.color);
            return seed;
        }
    };
};
#endif
