#ifndef __GUI_LAYER__
#define __GUI_LAYER__

#include "core.h"
#include "mesh.h"
#include "camera.h"
#include "light.h"

namespace glib
{

    namespace gui
    {
        void draw_transform_properties(Object3D *obj);
        void draw_light_properties(Light *l);
        void draw_point_light_properties(PointLight *l);
        void draw_directional_light_properties(PointLight *l);
        void draw_mesh_properties(Mesh *m);
        void draw_camera_properties(Camera *cam);
    }
}
#endif