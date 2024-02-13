#ifndef __GUI_LAYER__
#define __GUI_LAYER__

#include "core.h"
#include "mesh.h"
#include "camera.h"
#include "light.h"

GLIB_NAMESPACE_BEGIN

namespace gui
{
    void draw_transform_widget(Object3D *obj);
    void draw_light_widget(Light *l);
    void draw_point_light_widget(PointLight *l);
    void draw_directional_light_widget(PointLight *l);
    void draw_mesh_widget(Mesh *m);
    void draw_camera_widget(Camera *cam);
}
GLIB_NAMESPACE_END

#endif