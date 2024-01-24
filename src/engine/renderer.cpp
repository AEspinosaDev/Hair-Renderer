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
    {
        GLFW_CHECK();
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, m_context.OpenGLMajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, m_context.OpenGLMinor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, m_context.OpenGLProfile);

    m_window.ptr = glfwCreateWindow(m_window.extent.width, m_window.extent.height, m_window.title, NULL, NULL);
    if (!m_window.ptr)
    {
        glfwTerminate();
        GLFW_CHECK();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(m_window.ptr);
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW\n");
    }

    glfwSwapInterval(1);
}

void Renderer::init()
{

    if (!m_initQueue.functions.empty())
        m_initQueue.flush();

    enableDepthTest(m_settings.depthTest);
    enableDepthWrites(m_settings.depthWrites);
    
}

void Renderer::tick()
{
    while (!glfwWindowShouldClose(m_window.ptr))
    {
        double currentTime = glfwGetTime();
        m_time.delta = currentTime - m_time.last;
        m_time.last = currentTime;
        m_time.framesPerSecond = int(1.0 / m_time.delta);

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
    if (!m_cleanupQueue.functions.empty())
        m_cleanupQueue.flush();

    glfwDestroyWindow(m_window.ptr);
    glfwTerminate();
}
