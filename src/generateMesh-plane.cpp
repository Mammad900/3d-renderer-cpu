#include "generateMesh.h"
#include "material.h"
#include "object.h"
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

/// @brief Generates a plane mesh
/// @param material Material to assign all faces to
/// @param name Name of the mesh, used to reference it in the scene
/// @param subdivisionsX This and the next parameter define the number of faces in the plane. 
/// Usually 1x1 is enough, unless the face is very big, in which case it might be a good idea to increase it.
/// @param subdivisionsY 
/// @return Pointer to mesh object
shared_ptr<Mesh> createPlane(shared_ptr<Material> material, std::string name, uint16_t subdivisionsX, uint16_t subdivisionsY) {
    // Allocate the mesh and assign a label.
    shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
    mesh->label = name;

    // Calculate the number of vertices and faces.
    uint16_t numVertices = (subdivisionsX + 1) * (subdivisionsY + 1);
    uint16_t numFaces = subdivisionsX * subdivisionsY * 2;

    // Allocate memory for vertices and faces.
    mesh->faces = vector<Face>(numFaces);
    mesh->vertices = vector<Vertex>(numVertices);

    // Calculate step sizes for subdivisions.
    float stepX = 1.0f / subdivisionsX;
    float stepY = 1.0f / subdivisionsY;

    // Generate vertices.
    uint16_t vertexIndex = 0;
    for (uint16_t y = 0; y <= subdivisionsY; ++y) {
        for (uint16_t x = 0; x <= subdivisionsX; ++x) {
            float posX = -0.5f + x * stepX;
            float posY = -0.5f + y * stepY;

            mesh->vertices[vertexIndex] = {
                {posX, 0.0f, posY}, // Position
                {x / float(subdivisionsX), y / float(subdivisionsY)}, // UV
                {0.0f, -1.0f, 0.0f} // Normal
            };
            ++vertexIndex;
        }
    }

    // Generate faces.
    uint16_t faceIndex = 0;
    for (uint16_t y = 0; y < subdivisionsY; ++y) {
        for (uint16_t x = 0; x < subdivisionsX; ++x) {
            uint16_t topLeft = y * (subdivisionsX + 1) + x;
            uint16_t topRight = topLeft + 1;
            uint16_t bottomLeft = (y + 1) * (subdivisionsX + 1) + x;
            uint16_t bottomRight = bottomLeft + 1;

            // First triangle of the quad.
            mesh->faces[faceIndex++] = {topLeft, bottomLeft, topRight, material};

            // Second triangle of the quad.
            mesh->faces[faceIndex++] = {topRight, bottomLeft, bottomRight, material};
        }
    }

    return mesh;
}

shared_ptr<Mesh> makeExtrudedMesh(shared_ptr<Material> mat, std::string name, shared_ptr<Material> endCap, shared_ptr<Material> startCap, vector<sf::Vector2f> &vertices) {
    shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
    mesh->label = name;
    mesh->flatShading = true;
    
    bool hasEndCap = endCap != nullptr;
    bool hasStartCap = startCap != nullptr;
    int n = vertices.size();

    mesh->vertices = vector<Vertex>(n * 2);
    mesh->faces = vector<Face>(n*2 + (n - 2) * (hasEndCap + hasStartCap));
    
    for (int i = 0; i < n; i++) {
        // Vertices
        Vector2f pos = vertices[i];
        mesh->vertices[i] = Vertex{
            .position= {pos.x, pos.y, 0.5}
        };
        mesh->vertices[n+i] = Vertex{
            .position= {pos.x, pos.y, -0.5}
        };

        // Side faces
        uint16_t i2 = (i+1) % n;
        mesh->faces[2*i] = Face{
            .v1 = (uint16_t)(i),
            .v2 = (uint16_t)(i2),
            .v3 = (uint16_t)(n+i),
            .material = mat
        };
        mesh->faces[2*i+1] = Face{
            .v1 = (uint16_t)(i2),
            .v2 = (uint16_t)(n+i2),
            .v3 = (uint16_t)(n+i),
            .material = mat
        };
    }

    if(hasEndCap) {
        for (int i = 0; i < n-2; i++) {
            // End faces
            mesh->faces[2*n+i] = Face{
                .v1 = (uint16_t)(0),
                .v2 = (uint16_t)(i+2),
                .v3 = (uint16_t)(i+1),
                .material= endCap
            };
        }
    }
    if(hasStartCap) {
        int start = 2*n + hasEndCap*(n-2);
        for (int i = 0; i < n-2; i++) {
            // End faces
            mesh->faces[start+i] = Face{
                .v1 = (uint16_t)(n+i+1),
                .v2 = (uint16_t)(n+i+2),
                .v3 = (uint16_t)(n),
                .material= endCap
            };
        }
    }
    return mesh;
}