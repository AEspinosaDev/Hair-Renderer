#ifndef __MATERIAL__
#define __MATERIAL__

#include <unordered_map>
#include "shader.h"
#include "texture.h"

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

struct GraphicPipeline
{
    Shader *shader{nullptr};
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

    struct TextureData
    {
        Texture *texture;
        unsigned int slot;
        std::string uniformName;
    };
    std::unordered_map<unsigned int, TextureData> m_textures;

    virtual void upload_uniforms() const;

    virtual void setup_pipeline() const;

    virtual void bind_textures() const;
    
    virtual void unbind_textures() const;

public:
    Material(GraphicPipeline &pipeline) : m_pipeline(pipeline) {}
    Material(GraphicPipeline &pipeline, MaterialUniforms &uniforms) : m_pipeline(pipeline), m_uniforms(uniforms) {}

    inline virtual void set_texture(std::string uniformName, Texture *texture, unsigned int slot = 0)
    {
        m_pipeline.shader->bind();
        m_pipeline.shader->set_int(uniformName.c_str(), slot);
        m_pipeline.shader->unbind();
        m_textures[slot] = {texture, slot, uniformName};
    };

    inline virtual Texture *get_texture(unsigned int slot) { return m_textures[slot].texture; };

    inline virtual void set_uniforms(MaterialUniforms &uniforms) { m_uniforms = uniforms; }
    inline virtual MaterialUniforms get_uniforms() const { return m_uniforms; }

    inline virtual void set_pipeline(GraphicPipeline &pipeline) { m_pipeline = pipeline; }
    inline virtual GraphicPipeline get_pipeline() const { return m_pipeline; }

    /*
    Binds the material. Binds the shader, sets up the render state ,uploads uniforms and activates textures
    */
    virtual void bind() const;
    /*
   Unbinds the material
   */
    virtual void unbind() const;

   
};

// Some default materials

// class PhysicallyBasedMaterial: public Material{
//     public:
//      PhysicallyBasedMaterial(){}
//      set_albedo

// };

GLIB_NAMESPACE_END
#endif