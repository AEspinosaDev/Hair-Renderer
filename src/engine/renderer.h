#ifndef __RENDERER__
#define __RENDERER__

#include "core.h"

class Renderer
{
    GLFWwindow* m_window;
    
public:
    void run();

private:
    void init();
    void draw();
};

#endif