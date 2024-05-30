/*
    This file is part of OpenGL-StarterPack (GLSP), an open source OpenGL based framework
    that facilitates and speeds up demo and project creation by offering an abstraction to
    the basic objects of OpenGL as well as incluing the necessary libraries.

    MIT License

    Copyright (c) 2024 Antonio Espinosa Garcia

*/
#ifndef __SHADER__
#define __SHADER__

#include <unordered_map>
#include <sstream>
#include <fstream>
#include <vector>
#include "core.h"

GLIB_NAMESPACE_BEGIN

enum ShaderType
{
    LIT,
    UNLIT,
    COMPUTE,
    OTHER
};

/*
Struct that contains all graphic pipeline stages sources for compiling a shader.
Compute source is not included due to the fact that it is a stage independent to the graphic pipeline
(Check ComputeShader class).
*/
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

    std::unordered_map<const char *, int> m_uniformLocationCache; // Legacy uniform pipeline
    std::unordered_map<const char *, int> m_uniformBlockCache;    // Unifrom buffer pipeline

    virtual unsigned int get_uniform_location(const char *name);

    virtual unsigned int get_uniform_block(const char *name);

    virtual unsigned int compile(unsigned int type, const char *source);

    virtual unsigned int create_program(ShaderStageSource source);

    Shader(ShaderType t) : m_type(t) {} // Utility constructor for inheritance

public:
    /*
    Admitted files: glsl. This kind of file is custom made, it has the benefit of containing all stages in one single file
    */
    Shader(const char *filename, ShaderType t);

    Shader(ShaderStageSource src, ShaderType t);

    ~Shader() { cleanup(); }

    void bind() const;

    void unbind() const;

    inline void cleanup() { GL_CHECK(glDeleteProgram(m_ID)); }

    inline ShaderType get_type() { return m_type; }

#pragma region LEGACY UNIFORM PIPELINE

    void set_bool(const char *name, bool value) const;

    void set_int(const char *name, int value) const;

    void set_float(const char *name, float value);

    void set_mat4(const char *name, glm::mat4 value);

    void set_vec2(const char *name, glm::vec2 value);

    void set_vec3(const char *name, glm::vec3 value);

    void set_vec4(const char *name, glm::vec4 value);

#pragma endregion

#pragma region UBO PIPELINE

    void set_uniform_block(const char *name, unsigned int id);

#pragma endregion
    /*
    Useful if not using .glsl files. It compiles the .frag, .vert etc files and outputs a string ready to be attached to a ShaderStageSource for creating a shader.
    */
    static std::string parse_shader_stage(const char *filename);

    /*
    Parse .glsl file.
    */
    static ShaderStageSource parse_shader(const char *filename);
};
#pragma region COMPUTE SHADER

/*
GPGPU dedicated shader. This functionality is not related to the graphic pipeline by any means.
*/
class ComputeShader : public Shader
{
private:
    unsigned int compile(unsigned int type, const char *source);

    unsigned int create_program(const std::string &src);

public:
    ComputeShader(const char *filename);

    /*
    Launch the shader kernel in the GPU
    */
    void dispatch(Extent3D workGroupExtent, bool autoBarrier = true, unsigned int barrierType = GL_ALL_BARRIER_BITS) const;

    static void set_barrier(unsigned int barrierType = GL_ALL_BARRIER_BITS);

    /*
    GPU limitation query functions
    */
    static Extent3D query_max_workgroup_extent();
    static Extent3D query_max_workgroup_number();
    static int query_max_invocations();
};

#pragma endregion

GLIB_NAMESPACE_END

#endif