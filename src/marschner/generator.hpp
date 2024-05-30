#ifndef __GENERATOR__
#define __GENERATOR__
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <vector>

#include "../engine/core.h"
#include "../engine/shader.h"

USING_NAMESPACE_GLIB

typedef unsigned int uint;

namespace gen
{
    uint create_buffer(const std::vector<float> &data)
    {
        uint vbo;
        GL_CHECK(glGenBuffers(1, &vbo));
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vbo));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW));
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
        return vbo;
    }

    void tex2file(uint texture, int width, int height, const char *filename)
    {
        std::vector<float> data(width * height);

        GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture));
        GL_CHECK(glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, data.data()));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

        std::vector<unsigned char> imageData(width * height);
        for (int i = 0; i < width * height; ++i)
        {
            imageData[i] = static_cast<unsigned char>(data[i] * 255.0f); // Assuming the data is in the range [0.0, 1.0]
        }

        stbi_write_png(filename, width, height, 1, imageData.data(), width);
    }

    void compute_M()
    {

        // Create and set up compute M shader
        ComputeShader shader(SHADERS_PATH "m.glsl");

        // Create VBOS for the shader to work with
        std::vector<float> wi = {1.0f, 2.0f, 3.0f, 4.0f};
        std::vector<float> wo = {5.0f, 6.0f, 7.0f, 8.0f};

        uint wiBuffer = create_buffer(wi);
        uint woBUffer = create_buffer(wo);

        // Create a texture as output
        uint lut;
        GL_CHECK(glGenTextures(1, &lut));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, lut));
        GL_CHECK(glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, 4, 1));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

        // Compute
        GL_CHECK(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, wiBuffer));
        GL_CHECK(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, woBUffer));
        GL_CHECK(glBindImageTexture(0, lut, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F));

        shader.bind();
        shader.dispatch(
            {4, 1, 1},
            true,
            GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        // Save results in image
        tex2file(lut,4,1,"m_lut.png");
        std::cout << "Succesfully compute Marschner M Term..." << std::endl;

        // Cleanup
        shader.cleanup();
        GL_CHECK(glDeleteBuffers(1, &wiBuffer));
        GL_CHECK(glDeleteBuffers(1, &woBUffer));
        GL_CHECK(glDeleteTextures(1, &lut));
    }

}

#endif