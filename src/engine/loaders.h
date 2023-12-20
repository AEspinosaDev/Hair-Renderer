#ifndef __LOADERS__
#define __LOADERS__

#include <unordered_map>
#include <tiny_obj_loader.h>
#include "mesh.h"
#include "utils.h"

namespace OBJ_loader
{

    bool load_mesh(Mesh *const mesh, bool overrideGeometry, const char* fileName, bool importMaterials = false, bool calculateTangents = false);

}
namespace PLY_loader
{
    //TO DO
    bool load_mesh();

}

#endif