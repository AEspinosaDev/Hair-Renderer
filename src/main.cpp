
#include <iostream>
#include "hair_renderer.h"

int main(int, char **)
{
    Window window;
    window.extent = {1280,720};
    window.title = "Hair Viewer";
    HairRenderer renderer(window);
    try
    {
        renderer.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
 

    return EXIT_SUCCESS;

    return 0;
}
