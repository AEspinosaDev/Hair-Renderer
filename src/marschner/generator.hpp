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
    // AHEAD DECLARATION OF ALL FUNCTIONS ------>

    // Hair material parameters
    struct HairConstants;
    // Computes Marschnerr Longitudinal Term and saves it in a LUT
    void compute_M(const char *filename, uint resolution, std::vector<double> shifts, std::vector<double> betas);
    // Computes Marschnerr Azimuthal Term ands saves it in a LUT
    void compute_N(const char *filename, uint resolution, HairConstants &constants);
    // Computes any Marschnerr Azimuthal Term given its intersection point "p" (R,TT or TRT)
    double N_P(int p, double phi, double thD, double eta, double absorption);
    // Computes Marschnerr Azimuthal Term for R, the most simple.
    double N_0(double phi, double thD, double eta);
    // Computes Marschnerr Azimuthal Term for TRT, the most complex
    double N_2(double phi, double thD, HairConstants &constants);
    // Absorption term
    double T(double absorption, double gammaT);
    // Attenuation term
    double A(double absorption, int p, double h, double refraction, double etaPerpendicular, double etaParallel);

    // <-----------

    struct HairConstants
    {
        double aR = -10.0; //(-10º to -5º)
        double aTT = -aR * 0.5;
        double aTRT = -3.0 * aR * 0.5;

        double bR = 10.0; //(5º to 10º)
        double bTT = bR * 0.5;
        double bTRT = 2.0 * bR;

        double eta = 1.55;
        double absorption = 0.2;
        double eccentricity = 0.85;

        double kG = 0.5;
        double wc = 10.0;
        double Dh0 = 0.2;
        double DhM = 0.5;

        HairConstants() {}
        HairConstants(double shiftDeg, double betaDeg, double _eta = 1.55, double _absorption = 0.2) : aR(-shiftDeg),
                                                                                                 aTT(-shiftDeg * 0.5),
                                                                                                 aTRT(-3.0 * shiftDeg * 0.5),
                                                                                                 bR(betaDeg),
                                                                                                 bTT(betaDeg * 0.5),
                                                                                                 bTRT(betaDeg * 2.0),
                                                                                                 eta(_eta),
                                                                                                 absorption(_absorption) {}
    };

#pragma region Longitudinal Term
    void compute_M(const char *filename, uint resolution, std::vector<double> shifts, std::vector<double> betas)
    {
        // COnvert to radians
        auto deg2rad = [](double deg)
        { return deg * M_PI / 180.0; };

        shifts[0] = deg2rad(shifts[0]);
        shifts[1] = deg2rad(shifts[1]);
        shifts[2] = deg2rad(shifts[2]);

        betas[0] = deg2rad(betas[0]);
        betas[1] = deg2rad(betas[1]);
        betas[2] = deg2rad(betas[2]);


        const uint SIZE = resolution;
        struct RGBA
        {
            Color color;
            double cos_thD;
        };
        
        // matrix
        std::vector<std::vector<RGBA>> G(SIZE, std::vector<RGBA>(SIZE, {Color(0.0),0.0}));

        // Fetch max terms
        Color max{0.0};
        for (size_t x = 0; x < SIZE; x++)
            for (size_t y = 0; y < SIZE; y++)
            {
                double sin_thI = -1.0 + (x * 2.0) / SIZE;
                double sin_thR = -1.0 + (y * 2.0) / SIZE;
                double thI = asin(sin_thI);
                double thR = asin(sin_thR);
                double thH = (thR + thI) / 2.0;
                double thD = (thR - thI) / 2.0;

                double g_R = math::gaussian_distribution(betas[0], 2.0*(thH- shifts[0]));
                double g_TT = math::gaussian_distribution(betas[1], thH- shifts[1]);
                double g_TRT = math::gaussian_distribution(betas[2], thH- shifts[2]);

                // double g_R = math::energy_conservant_gaussian_distribution(betas[0], thI, thR - shifts[0] );
                // double g_TT = math::energy_conservant_gaussian_distribution(betas[1], thI, thR - shifts[1]);
                // double g_TRT = math::energy_conservant_gaussian_distribution(betas[2], thI, thR - shifts[2]);

                Color g = Color(g_R, g_TT, g_TRT);

                if (g.r > max.r) // Max R
                    max.r = g.r;
                if (g.g > max.g) // Max TT
                    max.g = g.g;
                if (g.b > max.b) // Max TRT
                    max.b = g.b;

                G[x][y] = {g,cos(thD)};
            }

#ifdef DEBUG_MODE
        DEBUG_LOG("Longitudinal Term");
        DEBUG_LOG("Max R = " << max.r);
        DEBUG_LOG("Max TT = " << max.g);
        DEBUG_LOG("Max TRT = " << max.b);
#endif
        const uint CHANNELS = 4;
        const uint TOTAL_SIZE = SIZE * SIZE * CHANNELS;
        std::vector<unsigned char> imageData(TOTAL_SIZE);
        // Normalize and save to file
        for (size_t x = 0; x < SIZE; x++)
            for (size_t y = 0; y < SIZE; y++)
            {
                Color norm_g = G[x][y].color / max;

                uint linearID = (y * SIZE + x) * CHANNELS;
                imageData[linearID + 0] = static_cast<unsigned char>(norm_g.r * 255.0);                  //R
                imageData[linearID + 1] = static_cast<unsigned char>(norm_g.g * 255.0);                  //TT
                imageData[linearID + 2] = static_cast<unsigned char>(norm_g.b * 255.0);                  //TRT
                imageData[linearID + 3] = static_cast<unsigned char>((G[x][y].cos_thD*0.5+0.5) * 255.0); //Cos Theta D
            }

        stbi_write_png(filename, SIZE, SIZE, CHANNELS, imageData.data(), SIZE * CHANNELS);

        std::cout << "Succesfully computed Marschner [M] Term..." << std::endl;
    }
#pragma endregion
#pragma region Azimuthal Term
#pragma endregion

    void compute_N(const char *filename, uint resolution, HairConstants &constants)
    {
        const uint SIZE = resolution;
        const uint CHANNELS = 3;
        const uint TOTAL_SIZE = SIZE * SIZE * CHANNELS;
        std::vector<unsigned char> imageData(TOTAL_SIZE);

        for (size_t x = 0; x < SIZE; x++)
            for (size_t y = 0; y < SIZE; y++)
            {
                double cos_phiD = -1.0 + (x * 2.0) / SIZE;
                double cos_thD = -1.0 + (y * 2.0) / SIZE;
                double phiD = acos(cos_phiD);
                double thD = acos(cos_thD);

                double n_R = N_P(0, phiD, thD, constants.eta, constants.absorption);
                double n_TT = N_P(1, phiD, thD, constants.eta, constants.absorption);
                double n_TRT = N_P(2, phiD, thD, constants.eta, constants.absorption);

                Color n = Color(n_R, n_TT, n_TRT);
                // Color n = Color(N_0(phiD,thD,constants.eta), n_TT, N_2(phiD,thD,constants));

                uint linearID = (y * SIZE + x) * CHANNELS;
                imageData[linearID + 0] = static_cast<unsigned char>(n.r * 255.0);
                imageData[linearID + 1] = static_cast<unsigned char>(n.g * 255.0);
                imageData[linearID + 2] = static_cast<unsigned char>(n.b * 255.0);
            }

        stbi_write_png(filename, SIZE, SIZE, CHANNELS, imageData.data(), SIZE * CHANNELS);

        std::cout << "Succesfully computed Marschner [N] Term..." << std::endl;
    }

#pragma endregion
#pragma region Azimuthal Term Aux

    double T(double absorption, double gammaT)
    {
        double l = 1.0 + cos(2.0 * gammaT);
        return exp(-2.0 * absorption * l);
    }

    double A(double absorption, int p, double h, double refraction,
             double etaPerpendicular, double etaParallel)
    {
        double gammaI = asin(h);

        // A(0; h) = F(h0; h00; gi)
        if (p == 0)
            return math::fresnel(etaPerpendicular, etaParallel, gammaI);

        // A(p; h) = ( (1 - F(h0; h00; gi) ) ^ 2 ) *
        //	( F(1 / h0; 1 / h00; gi) ^ (p - 1) ) * ( T(s0a; h) ^ p )
        double gammaT = asin(h / etaPerpendicular); // h0 sin gt = h

        double fresnel = math::fresnel(etaPerpendicular, etaParallel, gammaI);
        double invFrenel = math::fresnel(1.0 / etaPerpendicular,
                                         1.0 / etaParallel, gammaT);
        double t = T(absorption, gammaT);

#ifdef DEBUG_MODE
        // if (roots > 1 && fresnel * invFrenel != 1)
        //     DEBUG_LOG(" f " << fresnel << " if " << invFrenel);
#endif
        return (1.0 - fresnel) * (1.0 - fresnel) * pow(invFrenel, p - 1) * pow(t, p);
    }

    double N_P(int p, double phi, double thD, double eta, double absorption)
    {
        double etaPerpendicular = math::bravais_index(thD, eta);
        double etaParallel = (eta * eta) / etaPerpendicular;

        std::vector<double> roots = math::Equation::roots(p, etaPerpendicular, phi);
        double result = 0;

        for (int index = 0; index < roots[3]; index++)
        {
            double gammaI = roots[index];

            {
                double h = sin(gammaI);
                double finalAbsorption = A(absorption, p, h, eta, etaPerpendicular, etaParallel);
                double inverseDerivateAngle = math::Equation::inverse_first_derivative(p, etaPerpendicular, h);

                result += finalAbsorption * 2.0 * abs(inverseDerivateAngle); // 0.5 here
            }
        }

        return std::min(1.0, result);
    }

    double N_0(double phi, double thD, double eta)
    {

        double etaPerpendicular = math::bravais_index(thD, eta);
        double etaParallel = (eta * eta) / etaPerpendicular;
        double gammaI = -phi / 2.0;

        double h = sin(gammaI);

        double result = (sqrt(1.0 - h * h));

        result *= math::fresnel(etaPerpendicular, etaParallel, gammaI);

        return std::min(1.0, result);
    }

    double N_2(double phi, double thD, HairConstants &constants)
    {
        double causticLimit = constants.DhM;
        double causticWidth = constants.wc * M_PI / 180.0;
        double glintScale = constants.kG;
        double causticFade = constants.Dh0;

        double eta = constants.eta;
        double absorption = constants.absorption;

        double etaPerpendicular = math::bravais_index(thD, eta);
        double etaParallel = (eta * eta) / etaPerpendicular;

        double dH, t, hc, gammaC;

        if (etaPerpendicular < 2.0)
        {
            double c = asin(1.0 / etaPerpendicular);
            gammaC = sqrt((6.0 * 2.0 * c / M_PI - 2.0) / (3.0 * 8.0 * (2.0 * c / (M_PI * M_PI * M_PI))));
            hc = abs(sin(gammaC));

            double inverseDerivateAngle = math::Equation::inverse_second_derivative(2, etaPerpendicular, hc);
            dH = std::min(causticLimit, 2.0 * sqrt(2.0 * causticWidth * inverseDerivateAngle));
            t = 1.0;
        }
        else
        {
            gammaC = 0.0;
            hc = 0.0;
            dH = causticLimit;
            t = 1.0 - math::smoothstep(2.0, 2.0 + causticFade, etaPerpendicular);
        }

        gammaC = math::Equation::angle_polynomial(2, etaPerpendicular, hc);
        double result = N_P(2, phi, thD, eta, absorption);
        double finalAbsorption = A(absorption, 2, hc, eta, etaPerpendicular, etaParallel);

        result = result * (1 - t * math::gaussian_distribution(causticWidth, phi - gammaC) /
                                   math::gaussian_distribution(causticWidth, 0));
        result = result * (1 - t * math::gaussian_distribution(causticWidth, phi + gammaC) /
                                   math::gaussian_distribution(causticWidth, 0));

        result = result + t * glintScale * finalAbsorption * dH *
                              (math::gaussian_distribution(causticWidth, phi - gammaC) +
                               math::gaussian_distribution(causticWidth, phi + gammaC));

        return std::min(1.0, result);
    }


#pragma endregion

#pragma region Azimuthal Epic

// //Attenuattion Monochrome
// double Epic_AMono(double f, int p, double t){ //fresnel, Stage, Absorbtion
//   return (1-f)*(1-f)*pow(f,p-1)*pow(t,p);
// }
// // double Epic_ARGB(double f, int p, Color t){ //fresnel, Stage, Absorbtion
// //   return (1-f)*(1-f)*pow(f,p-1)*pow(t,Color(p));
// // }

// double Epic_N0(vec3 wi, vec3 wo, float cosPhi){
//   float cosHalfPhi = sqrt(0.5+0.5*cosPhi);

//   // return (0.25*cosHalfPhi)*fresnelSchlick(u_hair.ior,sqrt(0.5*(1+dot(wi,wo)))); //Frostbite
//   return (0.25*cosHalfPhi)*fresnelSchlick(u_hair.ior,sqrt(0.5+0.5*dot(wi,wo))) ; //Epic
// }
// double Epic_N1Mono(float sinThetaD, float cosThetaD, float cosPhi){

//   // float _ior = sqrt(u_hair.ior*u_hair.ior - sinThetaD* sinThetaD)/ cosThetaD; //Original
//   float _ior =  1.19 / cosThetaD + 0.36 * cosThetaD; //Fit EPIC
//   float a = 1/_ior;

//   float cosHalfPhi = sqrt(0.5+0.5*cosPhi); 
//   float h = 1 + a *(0.6-0.8*cosPhi) *  cosHalfPhi; //Fit EPIC

//   float power = sqrt(1-h*h*a*a)/(2*cosThetaD);

//   float D = exp(-3.65*cosPhi-3.98); //Intensity distribution
//   vec3 T = pow(u_hair.baseColor,vec3(power)); //Absortion
//   float F = fresnelSchlick(u_hair.ior,cosThetaD*sqrt(1-h*h)); //Fresnel

//   return A(F,1,T)*D;
// }
// // Azimuthal DOUBLE REFLECTION
// vec3 Epic_N2Mono(float sinThetaD, float cosThetaD, float cosPhi){
//   float h = sqrt(3.0)*0.5; //Por que si
// //   float _ior = sqrt(u_hair.ior*u_hair.ior - sinThetaD* sinThetaD)/ cosThetaD; //Original
// // float gamma = asin(h/_ior);
// //    vec3 T = exp(-2*u_hair.baseColor*(1+cos(2*gamma)));
//   //  float scale = clamp(1.5*(1-2.0*u_hair.roughness),0.0,1.0); //Frostbite scale term

//   float D = exp(17.0*cosPhi-16.78); //Intensity distribution
//    vec3 T = pow(u_hair.baseColor,vec3(0.8/cosThetaD));
//   float F = fresnelSchlick(u_hair.ior,acos(cosThetaD*sqrt(1-h*h))); //Fresnel CONSTANT ?
  
//   return A(F,2,T)*D;
// }

#pragma endregion

}

#endif