#ifndef __SETTINGS__
#define __SETTINGS__

#include "engine/core.h"
#include "engine/renderer.h"

struct UserInterfaceSettings
{
    ImVec2 padding;
    float windowBorder;
    float windowRounding;
};
struct HairSettings
{
    float thickness = 0.004f;
    // glm::vec3 color = glm::vec3(0.95f, 0.65f, 0.16f);
    glm::vec3 color = glm::vec3(0.6f, 0.078f, 0.078f);
    glm::vec3 specColor1 = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 specColor2 = color;
    float specPower1 = 210.0f;
    float specPower2 = 8.0f;
};
struct HeadSettings
{

    glm::vec3 skinColor = glm::vec3(0.84f, 0.25f, 0.125f);
};
struct GlobalSettings
{
    bool showUI{true};
    glm::vec3 ambientColor = glm::vec3(1.0f);
    float ambientStrength = 0.3f;
    Extent2D shadowExtent = {2048, 2048};
    unsigned int samples = 16;
    
};

#endif