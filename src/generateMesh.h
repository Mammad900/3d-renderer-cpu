#ifndef __GENERATEMESH_H__
#define __GENERATEMESH_H__

#include <string>
#include "object.h"

Mesh *createMesh(std::vector<Face> &faces, std::vector<Vertex> &vertices, std::string &name);

// Constructs a UV sphere as a Mesh pointer.
// Parameters:
//   stacks   - number of divisions along the latitude (vertical slices)
//   sectors  - number of divisions along the longitude (horizontal slices)
//   material - pointer to the Material to be assigned to each face
//
// Returns a pointer to a Mesh containing the sphere geometry.
Mesh* createSphere(Material* material, std::string name, uint16_t stacks, uint16_t sectors);
Mesh* createPlane(Material* material, std::string name, uint16_t subdivisionsX, uint16_t subdivisionsY);
Mesh *loadOBJ(const std::string &filename, Material *mat, std::string name);

#endif /* __GENERATEMESH_H__ */
