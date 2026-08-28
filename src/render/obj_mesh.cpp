#include "render/obj_mesh.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <sstream>

namespace {

struct FaceIndex {
    int position{0};
    int normal{0};
};

bool parseFaceIndex(const std::string& token, FaceIndex& output) {
    try {
        const std::size_t first = token.find('/');
        output.position = std::stoi(token.substr(0, first));
        if (first != std::string::npos) {
            const std::size_t second = token.find('/', first + 1u);
            if (second != std::string::npos && second + 1u < token.size()) {
                output.normal = std::stoi(token.substr(second + 1u));
            }
        }
    } catch (...) {
        return false;
    }
    return output.position != 0;
}

bool resolveIndex(int index, std::size_t size, std::size_t& resolved) {
    const long long candidate = index > 0
        ? static_cast<long long>(index - 1)
        : static_cast<long long>(size) + index;
    if (candidate < 0 || candidate >= static_cast<long long>(size)) {
        return false;
    }
    resolved = static_cast<std::size_t>(candidate);
    return true;
}

std::array<float, 3> faceNormal(const std::array<float, 3>& a,
                                const std::array<float, 3>& b,
                                const std::array<float, 3>& c) {
    const float ab_x = b[0] - a[0];
    const float ab_y = b[1] - a[1];
    const float ab_z = b[2] - a[2];
    const float ac_x = c[0] - a[0];
    const float ac_y = c[1] - a[1];
    const float ac_z = c[2] - a[2];
    std::array<float, 3> normal{
        ab_y * ac_z - ab_z * ac_y,
        ab_z * ac_x - ab_x * ac_z,
        ab_x * ac_y - ab_y * ac_x,
    };
    const float magnitude = std::sqrt(
        normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
    if (magnitude > 1e-12f) {
        for (float& component : normal) {
            component /= magnitude;
        }
    }
    return normal;
}

}  // namespace

bool loadObjMesh(const std::string& path, float position_scale,
                 ObjMesh& output, std::string& error) {
    output = {};
    if (!std::isfinite(position_scale) || position_scale <= 0.0f) {
        error = "OBJ position scale must be positive";
        return false;
    }
    std::ifstream input(path);
    if (!input) {
        error = "unable to open OBJ mesh: " + path;
        return false;
    }

    std::vector<std::array<float, 3>> positions;
    std::vector<std::array<float, 3>> normals;
    output.minimum.fill(std::numeric_limits<float>::infinity());
    output.maximum.fill(-std::numeric_limits<float>::infinity());
    std::string line;
    std::size_t line_number = 0;
    while (std::getline(input, line)) {
        ++line_number;
        std::istringstream stream(line);
        std::string record;
        stream >> record;
        if (record.empty() || record[0] == '#') {
            continue;
        }
        if (record == "v" || record == "vn") {
            std::array<float, 3> value{};
            if (!(stream >> value[0] >> value[1] >> value[2]) ||
                !std::isfinite(value[0]) || !std::isfinite(value[1]) ||
                !std::isfinite(value[2])) {
                error = "invalid OBJ vector at line " + std::to_string(line_number);
                return false;
            }
            if (record == "v") {
                for (float& component : value) {
                    component *= position_scale;
                }
                positions.push_back(value);
            } else {
                normals.push_back(value);
            }
            continue;
        }
        if (record != "f") {
            continue;
        }

        std::vector<FaceIndex> face;
        std::string token;
        while (stream >> token) {
            FaceIndex index;
            if (!parseFaceIndex(token, index)) {
                error = "invalid OBJ face index at line " + std::to_string(line_number);
                return false;
            }
            face.push_back(index);
        }
        if (face.size() < 3u) {
            error = "OBJ face has fewer than three vertices at line " +
                    std::to_string(line_number);
            return false;
        }

        for (std::size_t corner = 1u; corner + 1u < face.size(); ++corner) {
            const FaceIndex triangle_indices[3] = {face[0], face[corner], face[corner + 1u]};
            std::array<std::array<float, 3>, 3> triangle_positions{};
            std::array<std::array<float, 3>, 3> triangle_normals{};
            bool has_normals = true;
            for (std::size_t vertex = 0; vertex < 3u; ++vertex) {
                std::size_t position_index = 0;
                if (!resolveIndex(triangle_indices[vertex].position,
                                  positions.size(), position_index)) {
                    error = "OBJ position index is out of range at line " +
                            std::to_string(line_number);
                    return false;
                }
                triangle_positions[vertex] = positions[position_index];
                if (triangle_indices[vertex].normal == 0) {
                    has_normals = false;
                } else {
                    std::size_t normal_index = 0;
                    if (!resolveIndex(triangle_indices[vertex].normal,
                                      normals.size(), normal_index)) {
                        error = "OBJ normal index is out of range at line " +
                                std::to_string(line_number);
                        return false;
                    }
                    triangle_normals[vertex] = normals[normal_index];
                }
            }
            if (!has_normals) {
                const std::array<float, 3> normal = faceNormal(
                    triangle_positions[0], triangle_positions[1], triangle_positions[2]);
                triangle_normals.fill(normal);
            }
            for (std::size_t vertex = 0; vertex < 3u; ++vertex) {
                ObjMeshVertex packed;
                packed.position = triangle_positions[vertex];
                packed.normal = triangle_normals[vertex];
                packed.color = {0.18f, 0.42f, 0.58f};
                for (std::size_t axis = 0; axis < 3u; ++axis) {
                    output.minimum[axis] = std::min(output.minimum[axis], packed.position[axis]);
                    output.maximum[axis] = std::max(output.maximum[axis], packed.position[axis]);
                }
                output.indices.push_back(static_cast<unsigned int>(output.vertices.size()));
                output.vertices.push_back(packed);
            }
        }
    }

    if (positions.empty() || output.indices.empty()) {
        error = "OBJ mesh contains no renderable triangles";
        return false;
    }
    error.clear();
    return true;
}
