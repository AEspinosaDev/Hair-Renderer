#include <iostream>
#include <unistd.h>

#include "generator.hpp"

int main(int argc, char *argv[])
{
    try
    {
        LUTGen::HairConstants constants{};
        size_t textureSize{1024};

        // COMMAND LINE ARGUMENTS --->

        if (argc == 1)
            std::cout << "No arguments submitted, generating with default parameters..." << std::endl;

        printf("\n");

        for (int i = 0; i < argc; ++i)
        {
            std::string token(argv[i]);
            if (token == "--size")
            {
                textureSize = atoi(argv[i + 1]);
                i++;
                continue;
            }
            else if (token == "--shift")
            {
                size_t shift = atoi(argv[i + 1]);
                constants.aR = -shift;
                constants.aTT = -shift * 0.5;
                constants.aTRT = -3.0 * shift * 0.5;
                i++;
                continue;
            }
            else if (token == "--beta")
            {
                size_t beta = atoi(argv[i + 1]);
                constants.bR = beta;
                constants.bTT = beta * 0.5;
                constants.bTRT = 2.0 * beta;
                i++;
                continue;
            }
            else if (token == "--ior")
            {
                double ior = atof(argv[i + 1]);
                constants.eta = ior;
                i++;
                continue;
            }
            else if (token == "--abs")
            {
                double absorption = atof(argv[i + 1]);
                constants.absorption = absorption;
                i++;
                continue;
            }
            continue;
        }

        std::cout << "PARAMETERS" << std::endl;
        std::cout << "--------------------------" << std::endl;
        std::cout << "LUT Size  = " << textureSize << std::endl;
        std::cout << "Shift     = " << constants.aR << std::endl;
        std::cout << "Beta      = " << constants.bR << std::endl;
        std::cout << "IOR       = " << constants.eta << std::endl;
        std::cout << "Absoption = " << constants.absorption << std::endl;
        std::cout << "--------------------------" << std::endl;
        printf("\n");

        // Reading from texture =>
        // u = sin_I
        // v = sin_R
        LUTGen::compute_M(
            "m.png",
            textureSize,

            {constants.aR,
             constants.aTT,
             constants.aTRT},

            {constants.bR,
             constants.bTT,
             constants.bTRT});

        // Reading from texture =>
        // u = cos_PHI
        // v = cos_hD
        LUTGen::compute_N(
            "n.png",
            textureSize,
            constants);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

    return 0;
}
