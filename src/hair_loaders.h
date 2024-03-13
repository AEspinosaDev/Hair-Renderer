#ifndef __HAIR_LOADERS__
#define __HAIR_LOADERS__

#include "engine/loaders.h"

USING_NAMESPACE_GLIB

namespace hair_loaders
{
    void load_neural_hair(Mesh *const mesh, const char *fileName, Mesh *const skullMesh, bool preload = true, bool verbose = false, bool calculateTangents = false);

    void load_cy_hair(Mesh *const mesh, const char *fileName);
}

#endif