#ifndef __SETTINGS__
#define __SETTINGS__

#include "engine/core.h"
#include "engine/renderer.h"

#define MARSCHNER

struct UserInterfaceSettings
{
    ImVec2 padding;
    float windowBorder;
    float windowRounding;
};
struct HairSettings
{
    float thickness = 0.002f;
    float Rpower = 5.0f;
    float TRTpower = 15.0f;
#ifdef MARSCHNER
    // glm::vec3 baseColor = glm::vec3(
    //     68.0f / 255.0f,
    //     37.0f / 255.0f,
    //     10.0f / 255.0f);
    glm::vec3 baseColor = glm::vec3(
        20.0f / 255.0f,
        10.0f / 255.0f,
        2.0f / 255.0f);
    float TTpower = 1.0f;
    float roughness = 10.0f;
    float shift = 2.0f; // Deg
    float ior = 1.55f;

    bool r = true;
    bool tt = true;
    bool trt = true;

    bool scatter = true;
    bool colorScatter = false;
    float scatterExp = 500.0f;

    bool glints = true;

    bool occlusion = false;
#else
    // glm::vec3 color = glm::vec3(0.95f, 0.65f, 0.16f);
    glm::vec3 color = glm::vec3(0.6f, 0.078f, 0.078f);
    glm::vec3 specColor1 = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 specColor2 = color;
    float specPower1 = 210.0f;
    float specPower2 = 8.0f;
#endif
};
struct HeadSettings
{
    // glm::vec3 skinColor = glm::vec3(
    //     240.0f / 255.0f,
    //     184.0f / 255.0f,
    //     160.0f / 255.0f);
    glm::vec3 skinColor = glm::vec3(
        140.0f / 255.0f,
        140.0f / 255.0f,
        140.0f / 255.0f);
    bool useAlbedoTexture = false;
};
struct GlobalSettings
{
    bool showUI{true};
    glm::vec3 ambientColor = glm::vec3(1.0f);
    float ambientStrength = 0.4f;
    float enviromentRotation = 0.0f;
    bool useSkyboxIrradiance = false;
    Extent2D shadowExtent = {2048, 2048};
    unsigned int samples = 8;
    float exposure = 1.0;
    
};

#endif