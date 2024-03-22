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
    float thickness = 0.005f;
    glm::vec3 color = glm::vec3(0.27f,0.095f,0.02f);
    glm::vec3 specColor1 = glm::vec3(1.0f,1.0f,1.0f);
    glm::vec3 specColor2 = color;
    float specPower1 = 120.0f;
    float specPower2 = 120.0f;
    
};
struct HeadSettings{

    glm::vec3 skinColor = glm::vec3(0.84f,0.25f,0.125f);

};
struct GlobalSettings
{
    bool showUI{true};
    glm::vec3 ambientColor = glm::vec3(1.0f);
    float ambientStrength = 0.3f;
};

#endif