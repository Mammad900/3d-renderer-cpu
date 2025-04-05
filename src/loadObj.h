#ifndef __LOADOBJ_H__
#define __LOADOBJ_H__
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <SFML/Graphics.hpp>
#include "object.h"
#include "data.h"

using sf::Vector3f;

Mesh *loadOBJ(const std::string& filename) {
    std::ifstream file(filename);  // Like std::cin, but for a file
    if (!file) {
        std::cerr << "Failed to open " << filename << "\n";
        return NULL;
    }

    std::vector<Vertex> vertices;
    std::vector<Face> faces;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);  // lets us use >> on the line
        std::string prefix;
        iss >> prefix;
        if (prefix == "v")
        {
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
            faces.push_back(Face{.v1 = a, .v2 = b, .v3 = c, .material = &cubeMaterial});
        }
    }
    Mesh *mesh = new Mesh;

    mesh->n_vertices = vertices.size();
    mesh->n_faces = faces.size();
    mesh->vertices = new Vertex[vertices.size()];
    mesh->faces = new Face[faces.size()];

    std::copy(vertices.begin(), vertices.end(), mesh->vertices);
    std::copy(faces.begin(), faces.end(), mesh->faces);

    return mesh;
}

#endif /* __LOADOBJ_H__ */
