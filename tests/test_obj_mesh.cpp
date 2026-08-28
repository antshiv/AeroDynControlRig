#include "render/obj_mesh.h"

#include <cmath>
#include <cstdio>
#include <string>

int main() {
    ObjMesh mesh;
    std::string error;
    if (!loadObjMesh(ASR_DEFAULT_AIRCRAFT_MESH, 0.001f, mesh, error)) {
        std::fprintf(stderr, "mesh load failed: %s\n", error.c_str());
        return 1;
    }
    if (mesh.vertices.size() < 100u || mesh.indices.size() != mesh.vertices.size()) {
        std::fprintf(stderr, "mesh topology is unexpectedly small or inconsistent\n");
        return 2;
    }
    if (std::abs(mesh.minimum[0] + 0.2025f) > 1e-4f ||
        std::abs(mesh.maximum[0] - 0.2025f) > 1e-4f ||
        std::abs(mesh.maximum[2] - 0.136f) > 1e-4f) {
        std::fprintf(stderr, "mesh metre conversion or bounds are incorrect\n");
        return 3;
    }
    std::puts("AeroDyn generated aircraft mesh: all tests passed");
    return 0;
}
