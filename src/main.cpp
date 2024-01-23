
#include <iostream>
#include "hair_renderer.h"

int main(int, char **)
{

    

    HairRenderer renderer("Hair Viewer");
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
