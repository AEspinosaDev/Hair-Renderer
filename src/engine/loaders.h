#ifndef __LOADERS__
#define __LOADERS__

#include <random>
#include <thread>
#include <unordered_map>
#include <algorithm>
#include <tiny_obj_loader.h>
#include <tinyply.h>
#include <stb_image.h>
#include "mesh.h"
#include "utils.h"

GLIB_NAMESPACE_BEGIN

namespace loaders
{
    void load_OBJ(Mesh *const mesh, const char *fileName, bool importMaterials = false, bool calculateTangents = false);

    void load_PLY(Mesh *const mesh, const char *fileName, bool preload = true, bool verbose = false, bool calculateTangents = false);

    void load_image(Texture* const texture, const char *fileName);
    
}

GLIB_NAMESPACE_END

#endif