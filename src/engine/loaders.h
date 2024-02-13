#ifndef __LOADERS__
#define __LOADERS__

#include <random>
#include <unordered_map>
#include <algorithm>
#include <tiny_obj_loader.h>
#include <tinyply.h>
#include "mesh.h"
#include "utils.h"

GLIB_NAMESPACE_BEGIN

namespace loaders
{
    void load_OBJ(Mesh *const mesh, const char *fileName, bool importMaterials = false, bool calculateTangents = false);

    void load_PLY(Mesh *const mesh, const char *fileName, bool preload = true, bool verbose = false, bool calculateTangents = false);

    void load_neural_hair(Mesh *const mesh, const char *fileName, bool preload = true, bool verbose = false, bool calculateTangents = false);

    void augment_strands_density(Geometry &geom, std::vector<unsigned int> &roots, const int kNeighbours = 5, const int strandsPerNeighbourhood = 10);

    void load_cy_hair(Mesh *const mesh, const char *fileName);
}

GLIB_NAMESPACE_END

#endif