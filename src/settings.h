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
#ifdef MARSCHNER
    glm::vec3 baseColor = glm::vec3(
        68.0f / 255.0f,
        37.0f / 255.0f,
        10.0f / 255.0f);
    float Rpower = 5.0f;
    float TTpower = 1.0f;
    float TRTpower = 15.0f;
    float roughness = 0.4f;
    float shift = 0.12f; // In radians (-5º to -10º) => 0.088 to 0.17 //Not with epic 0.02 does fine
    float ior = 1.55f;

    bool r = true;
    bool tt = true;
    bool trt = true;

    bool scatter = true;
    bool colorScatter = true;
    float scatterExp = 500.0f;

    bool glints = false;

    bool occlusion = false;
#else
    // glm::vec3 color = glm::vec3(0.95f, 0.65f, 0.16f);
    glm::vec3 color = glm::vec3(0.6f, 0.078f, 0.078f);
    glm::vec3 specColor1 = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 specColor2 = color;
    float specPower1 = 210.0f;
    float specPower2 = 8.0f;
#endif

    // float cosTheta
};
struct HeadSettings
{
    glm::vec3 skinColor = glm::vec3(
        240.0f / 255.0f,
        184.0f / 255.0f,
        160.0f / 255.0f);
    bool useAlbedoTexture = false;
};
struct GlobalSettings
{
    bool showUI{true};
    glm::vec3 ambientColor = glm::vec3(1.0f);
    float ambientStrength = 0.25f;
    float enviromentRotation = 0.0f;
    bool useSkyboxIrradiance = false;
    Extent2D shadowExtent = {2048, 2048};
    unsigned int samples = 8;
    float exposure = 1.0;
    
};

#endif