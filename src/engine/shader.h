#ifndef __SHADER__
#define __SHADER__

#include <unordered_map>
#include <sstream>
#include <fstream>
#include "core.h"

GLIB_NAMESPACE_BEGIN

enum ShaderType
{
    LIT,
    UNLIT
};

struct ShaderStageSource
{
    std::string vertexBit;
    std::string fragmentBit;
    std::string geometryBit;
    std::string tesselationCtrlBit;
    std::string tesselationEvalBit;
};

class Shader
{
protected:
    unsigned int m_ID; // PROGRAM ID
    ShaderType m_type;
    
    std::unordered_map<const char *, int> m_uniformLocationCache; //Legacy uniform pipeline
    std::unordered_map<const char* , int> m_uniformBlockCache; //Unifrom buffer pipeline

public:
    Shader(const char *filename, ShaderType t);

    Shader(ShaderStageSource src, ShaderType t);

    ~Shader() {}

    void bind() const;

    void unbind() const;

    inline ShaderType get_type() { return m_type; }

#pragma region legacy_uniform_pipeline

    void set_bool(const char *name, bool value) const;

    void set_int(const char *name, int value) const;

    void set_float(const char *name, float value);

    void set_mat4(const char *name, glm::mat4 value);

    void set_vec3(const char *name, glm::vec3 value);

    void set_vec4(const char *name, glm::vec4 value);

#pragma endregion

#pragma region uniform_buffer_pipeline

    void set_uniform_block(const char *name, unsigned int id);

#pragma endregion

private:
    unsigned int get_uniform_location(const char *name);

    unsigned int get_uniform_block(const char* name);

    virtual unsigned int compile(unsigned int type, const char *source);

    virtual unsigned int create_program(ShaderStageSource source);

    static ShaderStageSource parse_shader(const char *filename);
};
GLIB_NAMESPACE_END

#endif