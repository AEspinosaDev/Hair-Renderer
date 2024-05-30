#include <iostream>
#include "generator.hpp"
#include <unistd.h>

int main(int, char **)
{
    try
    {
        if (!glfwInit())
        {
            GLFW_CHECK();
            exit(EXIT_FAILURE);
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        GLFWwindow *window;
        window = glfwCreateWindow(100, 100, "Dummy", NULL, NULL);
        if (!window)
        {
            glfwTerminate();
            GLFW_CHECK();
            exit(EXIT_FAILURE);
        }

        glfwMakeContextCurrent(window);
        if (glewInit() != GLEW_OK)
        {
            fprintf(stderr, "Failed to initialize GLEW\n");
        }

        glfwSwapInterval(0);

        gen::compute_M();

        glfwDestroyWindow(window);
        glfwTerminate();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

    return 0;
}
