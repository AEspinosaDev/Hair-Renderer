#ifndef __LIGHT__
#define __LIGHT__

#include "shader.h"
#include "object3D.h"

typedef enum LightType
{
    POINT,
    DIRECTIONAL,
    SPORT,
    AREA
};

class Light : public Object3D
{
protected:
    glm::vec3 m_color;
    float m_intensity;

    // Shadows
    struct Shadow
    {
        bool cast{true};
        float nearPlane{0.5f};
        float farPlane{100.0f};
        float fov{45.0f};
        glm::vec3 target{0.0f, 0.0f, 0.0f};

        float bias{0.1f};
        bool angleDependableBias{false};
        int pcfKernel{7};

        // Texture *map;
    };

    Shadow m_shadow;
    const LightType m_lighType;

public:
    Light(const char *name, LightType type, glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f), float intensity = 1.0f) : Object3D(name, glm::vec3{0.0f}, LIGHT), m_color(color),
                                                                                                                     m_intensity(intensity), m_lighType(type) {}
    virtual void cache_uniforms(Shader *shader) const = 0;

    virtual inline glm::vec3 get_color() const { return m_color; }
    virtual inline void set_color(glm::vec3 c) { m_color = c; }

    virtual inline float get_intensity() const { return m_intensity; }
    virtual inline void set_intensity(float i) { m_intensity = i; }

    virtual inline bool get_cast_shadows() const { return m_shadow.cast; }
    virtual inline void set_cast_shadows(bool o) { m_shadow.cast = o; }

    virtual inline glm::vec3 get_shadow_target() const { return m_shadow.target; }
    virtual inline void set_shadow_target(glm::vec3 o) { m_shadow.target = o; }

    virtual inline float get_shadow_near() const { return m_shadow.nearPlane; }
    virtual inline void set_shadow_near(float n) { m_shadow.nearPlane = n; }

    virtual inline float get_shadow_far() const { return m_shadow.farPlane; }
    virtual inline void set_shadow_far(float f) { m_shadow.farPlane = f; }

    virtual inline float get_shadow_fov() const { return m_shadow.fov; }
    virtual inline void set_shadow_fov(float f) { m_shadow.fov = f; }

    virtual inline float get_shadow_bias() const { return m_shadow.bias; }
    virtual inline void set_shadow_bias(float b) { m_shadow.bias = b; }

    virtual inline int get_shadow_pcf_kernel() const { return m_shadow.pcfKernel; }
    virtual inline void set_shadow_pcf_kernel(int k) { m_shadow.pcfKernel = k; }

    virtual inline bool get_angle_dependant_bias() const { return m_shadow.angleDependableBias; }
    virtual inline void set_angle_dependant_bias(bool o) { m_shadow.angleDependableBias = o; }

    virtual void user_interface_frame(bool displayName = false,bool includeParent = false);

    // Read only
    // virtual const Texture *const get_shadow_map() const { return m_shadow.map; }
    virtual LightType get_light_type() const { return m_lighType; }
};

// POINT LIGHT

class PointLight : public Light
{
    float m_effectArea;
    float m_decaying;

    static int INSTANCED_POINT_LIGHTS;

public:
    PointLight(glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f), float intensity = 0.1f) : Light("Point Light", LightType::POINT, color, intensity), m_effectArea(12.0f), m_decaying(1.0f) { PointLight::INSTANCED_POINT_LIGHTS++; }

    inline float get_area_of_effect() const { return m_effectArea; }
    inline void set_area_of_effect(float a) { m_effectArea = a; }

    inline float get_decaying() const { return m_decaying; }
    inline void set_decaying(float d) { m_decaying = d; }

    void cache_uniforms(Shader *shader) const;

    virtual void user_interface_frame(bool displayName = false,bool includeParent = false);


    inline static int get_number_of_instances() { return INSTANCED_POINT_LIGHTS; }
};

// DIRECTIONAL LIGHT

class DirectionalLight : public Light
{
    glm::vec3 m_direction;

    static int INSTANCED_DIRECTIONAL_LIGHTS;

public:
    DirectionalLight(glm::vec3 direction, glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f), float intensity = 1.0f) : Light("Directional Light", LightType::DIRECTIONAL, color, intensity), m_direction(direction) { DirectionalLight::INSTANCED_DIRECTIONAL_LIGHTS++; }

    inline glm::vec3 get_direction() const { return m_direction; }
    inline void set_direction(glm::vec3 d) { m_direction = d; }

    void cache_uniforms(Shader *shader) const;

    virtual void user_interface_frame(bool displayName = false,bool includeParent = false);


    inline static int get_number_of_instances() { return INSTANCED_DIRECTIONAL_LIGHTS; }
};

#endif