/*
    This file is part of OpenGL-StarterPack (GLSP), an open source OpenGL based framework
    that facilitates and speeds up demo and project creation by offering an abstraction to
    the basic objects of OpenGL as well as incluing the necessary libraries.

    MIT License

    Copyright (c) 2024 Antonio Espinosa Garcia

*/
#include "shader.h"

GLIB_NAMESPACE_BEGIN

Shader::Shader(const char *filename, ShaderType t) : m_type(t)
{
    m_ID = create_program(Shader::parse_shader(filename));
}

Shader::Shader(ShaderStageSource src, ShaderType t) : m_type(t)
{
    m_ID = create_program(src);
}

void Shader::bind() const
{
    GL_CHECK(glUseProgram(m_ID));
}

void Shader::unbind() const
{
    GL_CHECK(glUseProgram(0));
}

void Shader::set_bool(const char *name, bool value) const
{
    GL_CHECK(glUniform1i(glGetUniformLocation(m_ID, name), (int)value));
}

void Shader::set_int(const char *name, int value) const
{
    GL_CHECK(glUniform1i(glGetUniformLocation(m_ID, name), value));
}

void Shader::set_float(const char *name, float value)
{
    GL_CHECK(glUniform1f(get_uniform_location(name), value));
}

void Shader::set_mat4(const char *name, glm::mat4 value)
{
    GL_CHECK(glUniformMatrix4fv(get_uniform_location(name), 1, GL_FALSE,
                                &(value[0][0])));
}
void Shader::set_vec2(const char *name, glm::vec2 value)
{
    GL_CHECK(glUniform2fv(get_uniform_location(name), 1, &value[0]));
}
void Shader::set_vec3(const char *name, glm::vec3 value)
{

    GL_CHECK(glUniform3fv(get_uniform_location(name), 1, &value[0]));
}
void Shader::set_vec4(const char *name, glm::vec4 value)
{

    GL_CHECK(glUniform4fv(get_uniform_location(name), 1, &value[0]));
}

unsigned int Shader::get_uniform_location(const char *name)

{
    if (m_uniformLocationCache.find(name) != m_uniformLocationCache.end())
        return m_uniformLocationCache[name];
    GL_CHECK(int location = glGetUniformLocation(m_ID, name));

    if (location != -1)
        m_uniformLocationCache[name] = location;

    return location;
}

void Shader::set_uniform_block(const char *name, unsigned int id)
{
    GL_CHECK(glUniformBlockBinding(m_ID, get_uniform_block(name), id));
}

unsigned int Shader::get_uniform_block(const char *name)
{
    if (m_uniformBlockCache.find(name) != m_uniformBlockCache.end())
        return m_uniformBlockCache[name];

    GL_CHECK(int location = glGetUniformBlockIndex(m_ID, name));

    if (location != -1)
        m_uniformBlockCache[name] = location;

    return location;
}

unsigned int Shader::compile(unsigned int type, const char *source)
{
    unsigned int id = glCreateShader(type);

    GL_CHECK(glShaderSource(id, 1, &source, nullptr));
    GL_CHECK(glCompileShader(id));

    int result;
    GL_CHECK(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char *message = (char *)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        switch (type)
        {
        case GL_VERTEX_SHADER:
            ERR_LOG("Failed to compile VERTEX shader");
            break;
        case GL_FRAGMENT_SHADER:
            ERR_LOG("Failed to compile FRAGMENT shader");
            break;
        case GL_GEOMETRY_SHADER:
            ERR_LOG("Failed to compile GEOMETRY shader");
            break;
        case GL_TESS_CONTROL_SHADER:
            ERR_LOG("Failed to compile TESSELATION CONTROL shader");
            break;
        case GL_TESS_EVALUATION_SHADER:
            ERR_LOG("Failed to compile TESSELATION EVALUATION shader");
            break;
        }

        std::cout << message << std::endl;

        GL_CHECK(glDeleteShader(id));
        return 0;
    }

    return id;
}

unsigned int Shader::create_program(ShaderStageSource source)
{
    unsigned int program = glCreateProgram();

    unsigned int vs = 0;
    unsigned int fs = 0;
    unsigned int gs = 0;
    unsigned int tcs = 0;
    unsigned int tes = 0;

    if (!source.vertexBit.empty())
    {
        vs = compile(GL_VERTEX_SHADER, source.vertexBit.c_str());
        GL_CHECK(glAttachShader(program, vs));
    }
    if (!source.fragmentBit.empty())
    {
        fs = compile(GL_FRAGMENT_SHADER, source.fragmentBit.c_str());
        GL_CHECK(glAttachShader(program, fs));
    }
    if (!source.geometryBit.empty())
    {
        gs = compile(GL_GEOMETRY_SHADER, source.geometryBit.c_str());
        GL_CHECK(glAttachShader(program, gs));
    }
    if (!source.tesselationCtrlBit.empty())
    {
        tcs = compile(GL_TESS_CONTROL_SHADER, source.tesselationCtrlBit.c_str());
        GL_CHECK(glAttachShader(program, tcs));
    }
    if (!source.tesselationEvalBit.empty())
    {
        tes = compile(GL_TESS_EVALUATION_SHADER, source.tesselationEvalBit.c_str());
        GL_CHECK(glAttachShader(program, tes));
    }

    GL_CHECK(glLinkProgram(program));

    int linkStatus;
    GL_CHECK(glGetProgramiv(program, GL_LINK_STATUS, &linkStatus));
    if (linkStatus != GL_TRUE)
    {
        int infoLogLength;
        GL_CHECK(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength));
        std::vector<GLchar> infoLog(infoLogLength + 1);
        GL_CHECK(glGetProgramInfoLog(program, infoLogLength, nullptr, infoLog.data()));
        ERR_LOG("Error linking program: " << infoLog.data());
    }

    GL_CHECK(glValidateProgram(program));

    GL_CHECK(glDeleteShader(vs));
    GL_CHECK(glDeleteShader(fs));
    GL_CHECK(glDeleteShader(gs));
    GL_CHECK(glDeleteShader(tcs));
    GL_CHECK(glDeleteShader(tes));

    return program;
}

ShaderStageSource Shader::parse_shader(const char *filename)
{
    const std::string file(filename);
    std::ifstream stream(file);

    if (!stream.is_open())
    {
        ERR_LOG("Error opening file " + file);
    }

    const size_t MAX_STAGES = 5;
    enum class StageType
    {
        NONE = -1,
        VERTEX = 0,
        FRAGMENT = 1,
        GEOMETRY = 2,
        TESS_CTRL = 3,
        TESS_EVAL = 4

    };

    std::string line;
    std::stringstream ss[MAX_STAGES];
    StageType type = StageType::NONE;

    while (getline(stream, line))
    {
        if (line.find("#stage") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
            {
                type = StageType::VERTEX;
            }
            else if (line.find("fragment") != std::string::npos)
            {
                type = StageType::FRAGMENT;
            }
            else if (line.find("geometry") != std::string::npos)
            {
                type = StageType::GEOMETRY;
            }
            else if (line.find("control") != std::string::npos)
            {
                type = StageType::TESS_CTRL;
            }
            else if (line.find("eval") != std::string::npos)
            {
                type = StageType::TESS_EVAL;
            }
        }
        else
        {
            ss[(int)type] << line << '\n';
        }
    }
    return {ss[0].str().size() != 0 ? ss[0].str() : "",
            ss[1].str().size() != 0 ? ss[1].str() : "",
            ss[2].str().size() != 0 ? ss[2].str() : "",
            ss[3].str().size() != 0 ? ss[3].str() : "",
            ss[4].str().size() != 0 ? ss[4].str() : ""};
}

std::string Shader::parse_shader_stage(const char *filename)
{
    const std::string file(filename);
    std::ifstream stream(file);

    if (!stream.is_open())
    {
        ERR_LOG("Error opening file " + file);
    }
    std::string line;
    std::stringstream ss;

    while (getline(stream, line))
    {
        ss << line << '\n';
    }
    return ss.str();
}
#pragma region COMPUTE SHADER
ComputeShader::ComputeShader(const char *filename) : Shader(ShaderType::COMPUTE)
{
    m_ID = create_program(Shader::parse_shader_stage(filename));
}
unsigned int ComputeShader::compile(unsigned int type, const char *source)
{
    unsigned int id = glCreateShader(type);

    GL_CHECK(glShaderSource(id, 1, &source, nullptr));
    GL_CHECK(glCompileShader(id));

    int result;
    GL_CHECK(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char *message = (char *)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);

        ERR_LOG("Failed to compile COMPUTE shader");
        std::cout << message << std::endl;

        GL_CHECK(glDeleteShader(id));
        return 0;
    }
    return id;
}
unsigned int ComputeShader::create_program(const std::string &src)
{
    unsigned int program = glCreateProgram();
    unsigned int compute = 0;
    if (!src.empty())
    {
        compute = compile(GL_COMPUTE_SHADER, src.c_str());
        GL_CHECK(glAttachShader(program, compute));
    }
    GL_CHECK(glLinkProgram(program));

    int linkStatus;
    GL_CHECK(glGetProgramiv(program, GL_LINK_STATUS, &linkStatus));
    if (linkStatus != GL_TRUE)
    {
        int infoLogLength;
        GL_CHECK(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength));
        std::vector<GLchar> infoLog(infoLogLength + 1);
        GL_CHECK(glGetProgramInfoLog(program, infoLogLength, nullptr, infoLog.data()));
        ERR_LOG("Error linking program: " << infoLog.data());
    }

    GL_CHECK(glValidateProgram(program));

    GL_CHECK(glDeleteShader(compute));

    return program;
}
void ComputeShader::dispatch(Extent3D workGroupExtent, bool autoBarrier, unsigned int barrierType) const
{
    GL_CHECK(glDispatchCompute(workGroupExtent.width, workGroupExtent.height, workGroupExtent.depth));
    if (autoBarrier)
        ComputeShader::set_barrier(barrierType);
}

void ComputeShader::set_barrier(unsigned int barrierType)
{
    GL_CHECK(glMemoryBarrier(barrierType));
}
Extent3D ComputeShader::query_max_workgroup_extent()
{
    int size[3];
    for (int idx = 0; idx < 3; idx++)
    {
        GL_CHECK(glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, idx, &size[idx]));
    }
    return {size[0], size[1], size[2]};
}
Extent3D ComputeShader::query_max_workgroup_number()
{
    int count[3];
    for (int idx = 0; idx < 3; idx++)
    {
        GL_CHECK(glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, idx, &count[idx]));
    }
    return {count[0], count[1], count[2]};
}

int ComputeShader::query_max_invocations()
{
    int max;
    GL_CHECK(glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &max));
    return max;
}
#pragma endregion

GLIB_NAMESPACE_END