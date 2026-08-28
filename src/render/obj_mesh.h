#ifndef RENDER_OBJ_MESH_H
#define RENDER_OBJ_MESH_H

#include <array>
#include <string>
#include <vector>

struct ObjMeshVertex {
    std::array<float, 3> position{};
    std::array<float, 3> normal{};
    std::array<float, 3> color{};
};

struct ObjMesh {
    std::vector<ObjMeshVertex> vertices;
    std::vector<unsigned int> indices;
    std::array<float, 3> minimum{};
    std::array<float, 3> maximum{};
};

bool loadObjMesh(const std::string& path, float position_scale,
                 ObjMesh& output, std::string& error);

#endif
