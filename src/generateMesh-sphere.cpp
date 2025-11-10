#include "generateMesh.h"
#include "material.h"
#include "object.h"
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

Vector2f invertUV(Vector2f a, bool invertU, bool invertV) {
    return {
        invertU ? 1.0f - a.x : a.x,
        invertV ? 1.0f - a.y : a.y,
    };
}

/// @brief Generates a UV sphere.
/// @param material Material to assign to all faces
/// @param name Name of the mesh, used to reference it in the scene
/// @param stacks Number of vertical subdivisions (10-20 is good)
/// @param sectors Number of horizontal subdivisions (20 is good)
/// @return Pointer to mesh object
shared_ptr<Mesh> makeSphere(shared_ptr<Material> material, std::string name, uint16_t stacks, uint16_t sectors, bool invertU, bool invertV) {
    // Allocate the mesh and assign a label.
    shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
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
            
            mesh->vertices[index] = {
                .position = Vec3 { x, y, z },
                // UV coordinates: u is along the theta direction, v along the phi direction.
                .uv = invertUV(Vector2f{ static_cast<float>(j) / sectors, static_cast<float>(i) / stacks }, invertU, invertV),
                // For a unit sphere, the normal is the same as the position.
                .normal = -Vec3 { x, y, z }.normalized(),
            };
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

shared_ptr<Mesh> makeCylinder(shared_ptr<Material> material, std::string name, uint16_t sectors, shared_ptr<Material> endCap, shared_ptr<Material> startCap) {
    shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
    mesh->label = name;

    bool hasStartCap = startCap != nullptr;
    bool hasEndCap = endCap != nullptr;
    
    // Note: cap can't share vertices with main body because of normals
    mesh->vertices = vector<Vertex>(sectors * (2 + hasStartCap + hasEndCap) + hasStartCap + hasEndCap);
    mesh->faces = vector<Face>(sectors * (2 + hasStartCap + hasEndCap));

    uint16_t startCapVI = 0, endCapVI = 0;
    size_t startCapFI = 2 * sectors, endCapFI = 2 * sectors;
    if(hasStartCap) {
        startCapVI = 2 * sectors;
        endCapVI += sectors + 1;
        endCapFI += sectors;
        mesh->vertices[startCapVI] = Vertex{
            .position = {0, 0, 0.5},
            .uv = {0.5, 0.5},
            .normal = {0, 0, -1}
        };
    }
    if(hasEndCap) {
        endCapVI += 2 * sectors;
        mesh->vertices[endCapVI] = Vertex{
            .position = {0, 0, -0.5},
            .uv = {0.5, 0.5},
            .normal = {0, 0, 1}
        };
    }

    for (uint16_t i = 0; i < sectors; i++) {
        float u = (float)i / sectors;
        float θ = M_PI * 2.0f * u;
        float x = sinf(θ) * 0.5, y = cosf(θ) * 0.5;
        mesh->vertices[i] = Vertex{
            .position= {x, y, 0.5},
            .uv = {u, 0},
            .normal= {-x, -y, 0}
        };
        mesh->vertices[sectors + i] = Vertex{
            .position= {x, y, -0.5},
            .uv = {u, 1},
            .normal= {-x, -y, 0}
        };
        uint16_t iNext = (i + 1) % sectors;
        mesh->faces[i] = Face{
            iNext, 
            (uint16_t)(sectors + i), 
            i, material
        };
        mesh->faces[sectors + i] = Face{
            iNext, 
            (uint16_t)(sectors + iNext), 
            (uint16_t)(sectors + i), material
        };

        if(hasStartCap) {
            mesh->vertices[startCapVI+1+i] = Vertex{
                .position= {x, y, 0.5},
                .uv = {x + 0.5f, y + 0.5f},
                .normal= {0, 0, -1}
            };
            mesh->faces[startCapFI+i] = Face{
                startCapVI,
                (uint16_t)(startCapVI + 1 + iNext),
                (uint16_t)(startCapVI + 1 + i),
                startCap
            };
        }
        if(hasEndCap) {
            mesh->vertices[endCapVI+1+i] = Vertex{
                .position= {x, y, -0.5},
                .uv = {x + 0.5f, y + 0.5f},
                .normal= {0, 0, 1}
            };
            mesh->faces[endCapFI+i] = Face{
                endCapVI,
                (uint16_t)(endCapVI + 1 + i),
                (uint16_t)(endCapVI + 1 + iNext),
                endCap
            };
        }
    }

    return mesh;
}