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
};
struct HeadSettings{

    glm::vec3 skinColor = glm::vec3(1.0f,0.65f,0.46f);

};
struct GlobalSettings
{
    bool showUI{true};
    glm::vec3 ambientColor = glm::vec3(1.0f);
    float ambientStrength = 0.3f;
};

#endif