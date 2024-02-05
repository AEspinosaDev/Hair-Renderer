#ifndef __LOADERS__
#define __LOADERS__

#include <random>
#include <unordered_map>
#include <tiny_obj_loader.h>
#include <tinyply.h>
#include "mesh.h"
#include "utils.h"

namespace glib
{

    namespace loaders
    {
        bool load_OBJ(Mesh *const mesh, bool overrideGeometry, const char *fileName, bool importMaterials = false, bool calculateTangents = false);

        bool load_PLY(Mesh *const mesh, bool overrideGeometry, const char *fileName, bool preload = true, bool verbose = false, bool calculateTangents = false);

        bool load_NeuralHair(Mesh *const mesh, bool overrideGeometry, const char *fileName, bool preload = true, bool verbose = false, bool calculateTangents = false);
    
        void populate_strands();
    }

}

#endif