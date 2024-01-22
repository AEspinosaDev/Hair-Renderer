
#include <iostream>
#include <unistd.h>
#include <filesystem>

#include "engine/shader.h"
#include "engine/mesh.h"
#include "engine/loaders.h"
#include "engine/camera.h"
#include "engine/controller.h"

Camera *camera = new Camera();
Controller controller(camera);

#pragma region input
static void key_callback(GLFWwindow *w, int a, int b, int c, int d)
{
    // DEBUG_LOG("HOLA");
    controller.handle_keyboard(w, a, b, 0.1f);
    // DEBUG_LOG(camera->get_position().z);
}
static void mouse_callback(GLFWwindow *w, double x, double y)
{
    controller.handle_mouse(w,x,y);
}
static void resize_callback(GLFWwindow *w, int width, int height)
{
}
#pragma endregion

int main(int, char **)
{

    GLFWwindow *window;
    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(800, 600, "Hair Renderer", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

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

    //  m->set_geometry(g);
    OBJ_loader::load_mesh(m, false, "resources/models/cube.obj");

    m->generate_buffers();


    glDisable(GL_CULL_FACE);

    while (!glfwWindowShouldClose(window))
    {

        glClear(GL_COLOR_BUFFER_BIT);

        shader->bind();
        camera->set_projection(800,600);
        shader->set_mat4("u_viewProj", camera->get_projection() * camera->get_view());
        shader->set_mat4("u_model", m->get_model_matrix());

        m->draw();

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
