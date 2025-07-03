#include "generateMesh.h"

/// @brief Generates a plane mesh
/// @param material Material to assign all faces to
/// @param name Name of the mesh, used to reference it in the scene
/// @param subdivisionsX This and the next parameter define the number of faces in the plane. 
/// Usually 1x1 is enough, unless the face is very big, in which case it might be a good idea to increase it.
/// @param subdivisionsY 
/// @return Pointer to mesh object
Mesh* createPlane(Material* material, std::string name, uint16_t subdivisionsX, uint16_t subdivisionsY) {
    // Allocate the mesh and assign a label.
    Mesh* mesh = new Mesh;
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
                {0.0f, 1.0f, 0.0f} // Normal
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
