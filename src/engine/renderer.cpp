#include "renderer.h"

void Renderer::run()
{
    create_context();
    init();
    tick();
    cleanup();
}

void Renderer::create_context()
{
    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, m_context.OpenGLMajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, m_context.OpenGLMinor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, m_context.OpenGLProfile);

    m_window.ptr = glfwCreateWindow(m_window.extent.width, m_window.extent.height, m_window.title, NULL, NULL);
    if (!m_window.ptr)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // glfwSetKeyCallback(window, key_callback);
    // glfwSetCursorPosCallback(window, mouse_callback);

    glfwMakeContextCurrent(m_window.ptr);
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW\n");
    }

    glfwSwapInterval(1);
}

void Renderer::init()
{
}

void Renderer::tick()
{
    while (!glfwWindowShouldClose(m_window.ptr))
    {
        update();

        draw();

        glfwSwapBuffers(m_window.ptr);

        glfwPollEvents();
    }
}

void Renderer::update()
{
}

void Renderer::draw()
{
}

void Renderer::cleanup()
{
    glfwDestroyWindow(m_window.ptr);
    glfwTerminate();
}
