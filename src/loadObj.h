#ifndef __LOADOBJ_H__
#define __LOADOBJ_H__
#include "data.h"
#include "object.h"
#include "generateMesh.h"
#include <SFML/Graphics.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using sf::Vector3f;

Mesh *createMesh(
    std::vector<Face> &faces, std::vector<Vertex> &vertices, std::string &name
);

Mesh *loadOBJ(const std::string &filename, Material *mat, std::string name) {
    std::ifstream file(filename); // Like std::cin, but for a file
    if (!file) {
        std::cerr << "Failed to open " << filename << "\n";
        return NULL;
    }

    std::vector<Vertex> vertices;
    std::vector<Face> faces;

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
    return createMesh(faces, vertices, name);
}

#endif /* __LOADOBJ_H__ */
