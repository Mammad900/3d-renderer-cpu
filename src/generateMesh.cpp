#include "generateMesh.h"
#include <cmath>
#include <iostream>
#include <fstream>
#include <memory>

void bakeMeshNormals(Mesh &mesh) {
    for (auto &&face : mesh.faces) {
        Vertex &v1 = mesh.vertices[face.v1];
        Vertex &v2 = mesh.vertices[face.v2];
        Vertex &v3 = mesh.vertices[face.v3];
        Vec3 normal = (v3.position - v1.position)
                              .cross(v2.position - v1.position)
                              .normalized();
        v1.normal += normal;
        v2.normal += normal;
        v3.normal += normal;
    }

    for (auto &&vertex : mesh.vertices)
        if(vertex.normal.lengthSquared() == 0)
            std::cerr << "A vertex has zero normal! "
                         "This usually happens when a face is winded incorrectly "
                         "and cancels out another face's normal." << std::endl;
}

void bakeMeshBoundingBox(Mesh &mesh) {
    if (mesh.vertices.empty()) {
        mesh.boundingBox = AABB(); // empty box
        return;
    }
    Vec3 minV = mesh.vertices.front().position;
    Vec3 maxV = mesh.vertices.front().position;

    for (const auto& v : mesh.vertices) {
        const Vec3& p = v.position;
        if (p.x < minV.x) minV.x = p.x; else if (p.x > maxV.x) maxV.x = p.x;
        if (p.y < minV.y) minV.y = p.y; else if (p.y > maxV.y) maxV.y = p.y;
        if (p.z < minV.z) minV.z = p.z; else if (p.z > maxV.z) maxV.z = p.z;
    }
    mesh.boundingBox = AABB(minV, maxV);
}

shared_ptr<Mesh> loadOBJ(const std::filesystem::path &filename, shared_ptr<Material> mat, std::string name) {
    std::ifstream file(filename); // Like std::cin, but for a file
    if (!file) {
        std::cerr << "Failed to open " << filename << "\n";
        return NULL;
    }

    std::vector<Vertex> vertices;
    std::vector<Face> faces;
    uint16_t vt_n = 0;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line); // lets us use >> on the line
        std::string prefix;
        iss >> prefix;
        if (prefix == "v") {
            float x, y, z;
            iss >> x >> y >> z;
            vertices.push_back(Vertex{.position = {x, y, z}});
        }
        else if (prefix == "vt") {
            float u, v;
            iss >> u >> v;
            vertices[vt_n++].uv = {u,v};
        }
        else if (prefix == "f") {
            uint16_t a, b, c;
            iss >> a >> b >> c;
            a--;
            b--;
            c--;
            faces.push_back(
                Face{.v1 = a, .v2 = b, .v3 = c, .material = mat}
            );
        }
    }

    // BAKE NORMALS
    shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
    mesh->label = name;
    mesh->faces = faces;
    mesh->vertices = vertices;
    bakeMeshNormals(*mesh);
    return mesh;
}
