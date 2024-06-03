#include <iostream>
#include "generator.hpp"
#include <unistd.h>

int main(int, char **)
{
    try
    {
        LUTGen::HairConstants constants{};
        LUTGen::compute_M(
            "m.png",
            1024,

            {constants.aR,
             constants.aTT,
             constants.aTRT},

            {constants.bR,
             constants.bTT,
             constants.bTRT});

    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

    return 0;
}
