#include "shader.h"

GLIB_NAMESPACE_BEGIN

Shader::Shader(const char *filename, ShaderType t) : m_type(t)
{
    m_ID = create_program(parse_shader(filename));
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
        m_uniformLocationCache[name] = location;

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
            std::cout << "Failed to compile VERTEX shader" << std::endl;
            break;
        case GL_FRAGMENT_SHADER:
            std::cout << "Failed to compile FRAGMENT shader" << std::endl;
            break;
        case GL_GEOMETRY_SHADER:
            std::cout << "Failed to compile GEOMETRY shader" << std::endl;
            break;
        case GL_TESS_CONTROL_SHADER:
            std::cout << "Failed to compile TESSELATION CONTROL shader" << std::endl;
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
    unsigned int ts = 0;
    unsigned int tcs = 0;

    if (source.vertexBit != "")
    {
        vs = compile(GL_VERTEX_SHADER, source.vertexBit.c_str());
        GL_CHECK(glAttachShader(program, vs));
    }
    if (source.fragmentBit != "")
    {
        fs = compile(GL_FRAGMENT_SHADER, source.fragmentBit.c_str());
        GL_CHECK(glAttachShader(program, fs));
    }
    if (source.geometryBit != "")
    {
        gs = compile(GL_GEOMETRY_SHADER, source.geometryBit.c_str());
        GL_CHECK(glAttachShader(program, gs));
    }
    // if (source.tesselation__Bit)
    // {
    //     unsigned int vs = compile(GL_VERTEX_SHADER, source.vertexBit);
    //     GL_CHECK(glAttachShader(program, vs));
    // }

    GL_CHECK(glLinkProgram(program));
    GL_CHECK(glValidateProgram(program));

    GL_CHECK(glDeleteShader(vs));
    GL_CHECK(glDeleteShader(fs));
    GL_CHECK(glDeleteShader(gs));
    // GL_CHECK(glDeleteShader(fs));

    return program;
}

ShaderStageSource Shader::parse_shader(const char *filename)
{
    const std::string file(filename);
    // std::ifstream stream("/home/tony/Dev/OpenGL-Hair/resources/shaders/test.glsl");
    std::ifstream stream(file);

    if (!stream.is_open())
    {
        std::cerr << "Error opening file" << std::endl;
    }

    enum class StageType
    {
        NONE = -1,
        VERTEX = 0,
        FRAGMENT = 1,
        GEOMETRY = 2,
        TESSELATION_CTRL = 3,
        TESSELATION_EVAL = 4

    };

    std::string line;
    std::stringstream ss[5];
    StageType type = StageType::NONE;

    while (getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos)
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
        }
        else
        {
            ss[(int)type] << line << '\n';
        }
    }
    return {ss[0].str().size() != 0 ? ss[0].str() : "",
            ss[1].str().size() != 0 ? ss[1].str() : "",
            ss[2].str().size() != 0 ? ss[2].str() : "",
            "",
            ""};
}
GLIB_NAMESPACE_END
