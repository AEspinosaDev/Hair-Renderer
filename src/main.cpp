
#include <iostream>
#include <unistd.h>
#include <filesystem>

#include "engine/shader.h"
#include "engine/mesh.h"

int init_context(GLFWwindow *window);

int main(int, char **)
{

    GLFWwindow *window;
    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(640, 480, "Hair Renderer", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    glfwSwapInterval(1);

    // std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;
    chdir("/home/tony/Dev/OpenGL-Hair/");

    Shader *shader = new Shader("resources/shaders/test.glsl", UNLIT);

    Mesh *m = new Mesh();
    Geometry g;
    g.vertices = {{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                  {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                  {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
                  {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}}};

    g.indices = {0, 1, 2, 2, 3, 0};
    
    m->set_geometry(g);
    m->generate_buffers();

    glDisable(GL_CULL_FACE);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        shader->bind();

        m->draw();

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

int init_context(GLFWwindow *window)
{
    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(640, 480, "Hair Renderer", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    glfwSwapInterval(1);
}