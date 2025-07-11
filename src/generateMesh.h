#ifndef __GENERATEMESH_H__
#define __GENERATEMESH_H__

#include <string>
#include "object.h"

void bakeMeshNormals(Mesh &mesh);

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
Mesh *loadSTL(const std::string &filename, Material *mat, std::string name);

Mesh *makeRegularIcosahedron(std::string name, Material *mat);
Mesh *makeIcoSphere(std::string name, Material *mat, size_t subdivisionSteps);
Mesh *makeDodecahedron(std::string name, Material *mat, bool pentakis);
Mesh *makeTruncatedIcosahedron(std::string name, Material *mat, Material *matPentagons = nullptr);
Mesh *makeBall(std::string name, Material *mat, Material *matPentagons, size_t subdivisionSteps);

#endif /* __GENERATEMESH_H__ */
