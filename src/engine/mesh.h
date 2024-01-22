#ifndef __MESH__
#define __MESH__

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include "object3D.h"
#include "utils.h"

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

namespace std
{
    template <>
    struct hash<Vertex>
    {
        size_t operator()(Vertex const &vertex) const
        {
            size_t seed = 0;
            utils::hash_combine(seed, vertex.position, vertex.normal, vertex.tangent, vertex.uv, vertex.color);
            return seed;
        }
    };
};

struct Geometry
{
    size_t triangles;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
};

class Mesh : public Object3D
{
    unsigned int m_vao;
    Geometry m_geometry;
    bool m_indexed;

    static int INSTANCED_MESHES;

public:
    Mesh() : Object3D("Mesh", {0.0f, 0.0f, 0.0f}, Object3DType::MESH) { Mesh::INSTANCED_MESHES++; }

    inline unsigned int get_buffer_id() const { return m_vao; }
    inline bool is_indexed() const { return m_indexed; }

    void set_geometry(Geometry &g);

    void generate_buffers();

    void draw(GLenum drawingPrimitive = GL_TRIANGLES) const;

    inline static int get_number_of_instances(){ return INSTANCED_MESHES;}
};

#endif
