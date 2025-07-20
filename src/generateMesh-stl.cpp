#include "generateMesh.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include <iostream>

Mesh* loadSTL(const std::string& filename, Material* mat, std::string name) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open " << filename << "\n";
        return nullptr;
    }

    // Check if the file is ASCII or binary
    std::string header(80, '\0');
    file.read(&header[0], 80);
    file.seekg(0);

    bool isBinary = header.find("solid") != 0;

    std::vector<Vertex> vertices;
    std::vector<Face> faces;

    if (isBinary) {
        // Binary STL format
        file.seekg(80); // Skip header
        uint32_t numTriangles;
        file.read(reinterpret_cast<char*>(&numTriangles), sizeof(numTriangles));

        for (uint32_t i = 0; i < numTriangles; ++i) {
            Vec3 normal, v1, v2, v3;
            file.read(reinterpret_cast<char*>(&normal), sizeof(Vec3));
            file.read(reinterpret_cast<char*>(&v1), sizeof(Vec3));
            file.read(reinterpret_cast<char*>(&v2), sizeof(Vec3));
            file.read(reinterpret_cast<char*>(&v3), sizeof(Vec3));

            uint16_t attributeByteCount;
            file.read(reinterpret_cast<char*>(&attributeByteCount), sizeof(attributeByteCount));

            uint16_t index1 = vertices.size();
            uint16_t index2 = index1 + 1;
            uint16_t index3 = index2 + 1;

            vertices.push_back(Vertex{.position = v1, .normal = normal});
            vertices.push_back(Vertex{.position = v2, .normal = normal});
            vertices.push_back(Vertex{.position = v3, .normal = normal});

            faces.push_back(Face{.v1 = index1, .v2 = index2, .v3 = index3, .material = mat});
        }
    } else {
        // ASCII STL format
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string keyword;
            iss >> keyword;

            if (keyword == "facet") {
                Vec3 normal;
                iss >> keyword >> normal.x >> normal.y >> normal.z; // "normal x y z"

                std::getline(file, line); // Skip "outer loop"

                Vec3 v1, v2, v3;
                for (int i = 0; i < 3; ++i) {
                    std::getline(file, line);
                    std::istringstream vertexStream(line);
                    vertexStream >> keyword >> v1.x >> v1.y >> v1.z; // "vertex x y z"
                    if (i == 0) v1 = v1;
                    if (i == 1) v2 = v1;
                    if (i == 2) v3 = v1;
                }

                std::getline(file, line); // Skip "endloop"
                std::getline(file, line); // Skip "endfacet"

                uint16_t index1 = vertices.size();
                uint16_t index2 = index1 + 1;
                uint16_t index3 = index2 + 1;

                vertices.push_back(Vertex{.position = v1, .normal = normal});
                vertices.push_back(Vertex{.position = v2, .normal = normal});
                vertices.push_back(Vertex{.position = v3, .normal = normal});

                faces.push_back(Face{.v1 = index1, .v2 = index2, .v3 = index3, .material = mat});
            }
        }
    }

    Mesh* mesh = new Mesh(name, vertices, faces, true);
    return mesh;
}
