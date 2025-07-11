#include "generateMesh.h"

/// @brief Generates a UV sphere.
/// @param material Material to assign to all faces
/// @param name Name of the mesh, used to reference it in the scene
/// @param stacks Number of vertical subdivisions (10-20 is good)
/// @param sectors Number of horizontal subdivisions (20 is good)
/// @return Pointer to mesh object
Mesh* createSphere(Material* material, std::string name, uint16_t stacks, uint16_t sectors) {
    // Allocate the mesh and assign a label.
    Mesh* mesh = new Mesh;
    mesh->label = name;
    
    // Calculate number of vertices:
    // There will be (stacks + 1) rows and (sectors + 1) columns of vertices.
    uint16_t numVertices = (stacks + 1) * (sectors + 1);
    mesh->vertices = vector<Vertex>(numVertices);
    
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
    mesh->faces = vector<Face>(numFaces);

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
