#ifndef __UTILS__
#define __UTILS__

#include "core.h"

namespace utils
{

    glm::vec3 get_tangent_gram_smidt(glm::vec3 &p1, glm::vec3 &p2, glm::vec3 &p3, glm::vec2 &uv1, glm::vec2 &uv2, glm::vec2 &uv3, glm::vec3 normal);

    template <typename T, typename... Rest>
    void hash_combine(std::size_t &seed, const T &v, const Rest &...rest)
    {
        seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        (hash_combine(seed, rest), ...);
    }

}

#endif