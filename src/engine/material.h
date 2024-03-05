#ifndef __MATERIAL__
#define __MATERIAL__

#include <unordered_map>
#include "shader.h"

GLIB_NAMESPACE_BEGIN

struct PipelineState
{
    bool cullFace{false};
    bool depthWrites{true};
    bool depthTest{true};
    DepthFuncType depthFunction{LESS};
    bool blending{false};
    BlendFuncType blendingFuncSRC{ONE_MINUS_SRC_A};
    BlendFuncType blendingFuncDST{ONE};
    BlendOperationType blendingOperation{ADD};
    bool alphaToCoverage{false};
    bool alphaTest{false};
};

struct GraphicPipeline{
    Shader* shader;
    PipelineState state{};
};
struct MaterialUniforms
{
    std::unordered_map<std::string, glm::vec3> vec3Types;
    std::unordered_map<std::string, glm::vec4> vec4Types;
    std::unordered_map<std::string, float> floatTypes;
    std::unordered_map<std::string, int> intTypes;
    std::unordered_map<std::string, bool> boolTypes;
    std::unordered_map<std::string, glm::mat4> mat4Types;
};

class Material
{
protected:
    GraphicPipeline m_pipeline;
    MaterialUniforms m_uniforms;
    // std::unordered_map<int,Texture* > m_textures;

public:
    Material(GraphicPipeline& pipeline) : m_pipeline(pipeline) {}
    Material(GraphicPipeline& pipeline, MaterialUniforms& uniforms) : m_pipeline(pipeline), m_uniforms(uniforms) {}
    // Material(Shader const *shader, MaterialUniforms uniforms, std::unordered_map<int, Texture *> textures) : m_shader(shader), m_uniforms(uniforms), m_textures(textures) {}

    // virtual void set_texture(label, texture);
    // virutal Texture* get_texture(label);

    inline virtual void set_uniforms(MaterialUniforms& uniforms){ m_uniforms = uniforms;}
    inline virtual MaterialUniforms get_uniforms() const { return m_uniforms;}

    inline virtual void set_pipeline(GraphicPipeline& pipeline){ m_pipeline = pipeline;}
    inline virtual GraphicPipeline get_pipeline() const { return m_pipeline;}


    virtual void upload_uniforms() const;
    virtual void setup_pipeline();
    virtual void bind_textures();

};

//Some default materials

// class PhysicallyBasedMaterial: public Material{
//     public:
//      PhysicallyBasedMaterial(){}
//      set_albedo




// };

GLIB_NAMESPACE_END
#endif