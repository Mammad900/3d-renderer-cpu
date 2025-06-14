#include "generateMesh.h"
#include <cmath>
#include <iostream>
#include <fstream>

Mesh *createMesh(std::vector<Face> &faces, std::vector<Vertex> &vertices, std::string &name) {
    for (auto &&face : faces) {
        Vertex &v1 = vertices[face.v1];
        Vertex &v2 = vertices[face.v2];
        Vertex &v3 = vertices[face.v3];
        Vector3f normal = (v3.position - v1.position)
                              .cross(v2.position - v1.position)
                              .normalized();
        v1.normal += normal;
        v2.normal += normal;
        v3.normal += normal;
    }

#ifndef NDEBUG
    for (auto &&vertex : vertices)
        if(vertex.normal.lengthSquared() == 0)
            std::cerr << "A vertex has zero normal! "
                         "This usually happens when a face is winded incorrectly "
                         "and cancels out another face's normal." << std::endl;
#endif

    // CREATE MESH OBJECT
    Mesh *mesh = new Mesh;

    mesh->n_vertices = vertices.size();
    mesh->n_faces = faces.size();
    mesh->vertices = new Vertex[vertices.size()];
    mesh->faces = new Face[faces.size()];
    mesh->label = name;

    std::copy(vertices.begin(), vertices.end(), mesh->vertices);
    std::copy(faces.begin(), faces.end(), mesh->faces);

    return mesh;
}


// Constructs a UV sphere as a Mesh pointer.
// Parameters:
//   stacks   - number of divisions along the latitude (vertical slices)
//   sectors  - number of divisions along the longitude (horizontal slices)
//   material - pointer to the Material to be assigned to each face
//
// Returns a pointer to a Mesh containing the sphere geometry.
Mesh* createSphere(Material* material, std::string name, uint16_t stacks, uint16_t sectors) {
    // Allocate the mesh and assign a label.
    Mesh* mesh = new Mesh;
    mesh->label = name;
    
    // Calculate number of vertices:
    // There will be (stacks + 1) rows and (sectors + 1) columns of vertices.
    uint16_t numVertices = (stacks + 1) * (sectors + 1);
    mesh->n_vertices = numVertices;
    mesh->vertices = new Vertex[numVertices];
    
    // Create vertices using spherical coordinates.
    // The sphere is assumed to have a radius of 1.
    // "phi" is the angle from the positive Y-axis, and "theta" is the angle around the Y-axis.
    for (uint16_t i = 0; i <= stacks; ++i) {
        float phi = M_PI * i / stacks;  // 0..pi
        for (uint16_t j = 0; j <= sectors; ++j) {
            float theta = 2.0f * M_PI * j / sectors;  // 0..2pi

            // Compute the vertex index.
            uint16_t index = i * (sectors + 1) + j;
            
            // Spherical to Cartesian conversion.
            float x = std::sin(phi) * std::cos(theta);
            float y = std::cos(phi);
            float z = std::sin(phi) * std::sin(theta);
            
            // Set position. (Assuming Vector3f can be constructed from an initializer list.)
            mesh->vertices[index].position = Vector3f { x, y, z };
            // For a unit sphere, the normal is the same as the position.
            mesh->vertices[index].normal = -Vector3f { x, y, z }.normalized();
            // UV coordinates: u is along the theta direction, v along the phi direction.
            mesh->vertices[index].uv = Vector2f { static_cast<float>(j) / sectors, static_cast<float>(i) / stacks };
        }
    }
    
    // Calculate the number of triangular faces.
    // Each quad on the sphere (except at the poles) is split into 2 triangles.
    // Using the common algorithm:
    //   For each stack row (except the last),
    //   For each sector, add the triangles if they are not degenerate.
    // The total face count comes out to 2 * sectors * (stacks - 1).
    uint16_t numFaces = 2 * sectors * (stacks - 1);
    mesh->n_faces = numFaces;
    mesh->faces = new Face[numFaces];

    // Build faces (triangles) using indices of the vertices.
    // We use counter-clockwise winding order (when looking from the outside)
    // which is standard in most rendering engines.
    int faceIndex = 0;
    for (uint16_t i = 0; i < stacks; ++i) {
        for (uint16_t j = 0; j < sectors; ++j) {
            // Indices for the current quad.
            uint16_t k1 = i * (sectors + 1) + j;       // current row, current column
            uint16_t k2 = (i + 1) * (sectors + 1) + j;   // next row, current column

            // For the top stack, skip the first triangle (avoid degenerate cap)
            if (i != 0) {
                // First triangle: vertices k1, k2, k1+1.
                mesh->faces[faceIndex].v1 = k1;
                mesh->faces[faceIndex].v3 = k2;
                mesh->faces[faceIndex].v2 = k1 + 1;
                // Assign the material.
                mesh->faces[faceIndex].material = material;
                ++faceIndex;
            }
            // For the bottom stack, skip the second triangle.
            if (i != (stacks - 1)) {
                // Second triangle: vertices k1+1, k2, k2+1.
                mesh->faces[faceIndex].v1 = k1 + 1;
                mesh->faces[faceIndex].v3 = k2;
                mesh->faces[faceIndex].v2 = k2 + 1;
                // Assign the material.
                mesh->faces[faceIndex].material = material;
                ++faceIndex;
            }
        }
    }
    
    return mesh;
}

Mesh* createPlane(Material* material, std::string name, uint16_t subdivisionsX, uint16_t subdivisionsY) {
    // Allocate the mesh and assign a label.
    Mesh* mesh = new Mesh;
    mesh->label = name;

    // Calculate the number of vertices and faces.
    uint16_t numVertices = (subdivisionsX + 1) * (subdivisionsY + 1);
    uint16_t numFaces = subdivisionsX * subdivisionsY * 2;

    // Allocate memory for vertices and faces.
    mesh->vertices = new Vertex[numVertices];
    mesh->faces = new Face[numFaces];
    mesh->n_vertices = numVertices;
    mesh->n_faces = numFaces;

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

