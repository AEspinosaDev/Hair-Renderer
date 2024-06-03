#ifndef __GENERATOR__
#define __GENERATOR__
#define STB_IMAGE_WRITE_IMPLEMENTATION

#define DEBUG_MODE
#define DEBUG_LOG(msg)                 \
    {                                  \
        std::cout << msg << std::endl; \
    }

#include <vector>
#include <math.h>
#include <stb_image_write.h>
#include "gmath.hpp"

typedef unsigned int uint;
using namespace math;

namespace LUTGen
{
    struct HairConstants
    {
        double aR = -10.0;
        double aTT = -aR * 0.5;
        double aTRT = -3.0 * aR * 0.5;

        double bR = 5.0;
        double bTT = bR * 0.5;
        double bTRT = 2.0 * bR;

        double eta = 1.55;
        double absorption = 0.2;
        double eccentricity = 0.85;

        double kG = 0.5;
        double wc = 10.0;
        double Dh0 = 0.2;
        double DhM = 0.5;
    };

    void compute_M(const char *filename, uint resolution, std::vector<double> shifts, std::vector<double> betas)
    {

        const uint SIZE = resolution;
        // matrix
        std::vector<std::vector<Color>> G(SIZE, std::vector<Color>(SIZE, Color(0.0)));

        // Fetch max terms
        Color max{0.0};
        for (size_t x = 0; x < SIZE; x++)
            for (size_t y = 0; y < SIZE; y++)
            {
                double sin_thI = -1.0 + (x * 2.0) / SIZE;
                double sin_thR = -1.0 + (y * 2.0) / SIZE;
                double thI = (180.0 * asin(sin_thI) / M_PI);
                double thR = (180.0 * asin(sin_thR) / M_PI);
                double thH = (thR + thI) / 2.0;

                double thH_R = thH - shifts[0];
                double thH_TT = thH - shifts[1];
                double thH_TRT = thH - shifts[2];

                double g_R = math::gaussian_distribution(betas[0], thH_R);
                double g_TT = math::gaussian_distribution(betas[1], thH_TT);
                double g_TRT = math::gaussian_distribution(betas[2], thH_TRT);

                Color g = Color(g_R, g_TT, g_TRT);

                if (g.r > max.r) // Max R
                    max.r = g.r;
                if (g.g > max.g) // Max TT
                    max.g = g.g;
                if (g.b > max.b) // Max TRT
                    max.b = g.b;

                G[x][y] = g;
            }

#ifdef DEBUG_MODE
        DEBUG_LOG("Max R = " << max.r*255.0 << "/255");
        DEBUG_LOG("Max TT = " << max.g*255.0 << "/255");
        DEBUG_LOG("Max TRT = " << max.b*255.0 << "/255");
#endif
        const uint CHANNELS = 3;
        const uint TOTAL_SIZE = SIZE * SIZE * CHANNELS;
        std::vector<unsigned char> imageData(TOTAL_SIZE);
        // Normalize and save to file
        for (size_t x = 0; x < SIZE; x++)
            for (size_t y = 0; y < SIZE; y++)
            {
                Color norm_g = G[x][y] / max;

                uint linearID = (y * SIZE + x) * CHANNELS;
                imageData[linearID + 0] = static_cast<unsigned char>(norm_g.r * 255.0);
                imageData[linearID + 1] = static_cast<unsigned char>(norm_g.g * 255.0);
                imageData[linearID + 2] = static_cast<unsigned char>(norm_g.b * 255.0);
            }

        stbi_write_png(filename, SIZE, SIZE, CHANNELS, imageData.data(), SIZE * CHANNELS);

        std::cout << "Succesfully computed Marschner M Term..." << std::endl;
    }
}

#endif