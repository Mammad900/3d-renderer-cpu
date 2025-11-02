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
shared_ptr<Mesh> createSphere(shared_ptr<Material> material, std::string name, uint16_t stacks, uint16_t sectors, bool invertU, bool invertV);
shared_ptr<Mesh> createPlane(shared_ptr<Material> material, std::string name, uint16_t subdivisionsX, uint16_t subdivisionsY);
shared_ptr<Mesh> loadOBJ(const std::filesystem::path &filename, shared_ptr<Material> mat, std::string name);
shared_ptr<Mesh> loadSTL(const std::filesystem::path &filename, shared_ptr<Material> mat, std::string name);

shared_ptr<Mesh> makeRegularIcosahedron(std::string name, shared_ptr<Material> mat);
shared_ptr<Mesh> makeIcoSphere(std::string name, shared_ptr<Material> mat, size_t subdivisionSteps);
shared_ptr<Mesh> makeDodecahedron(std::string name, shared_ptr<Material> mat, bool pentakis);
shared_ptr<Mesh> makeTruncatedIcosahedron(std::string name, shared_ptr<Material> mat, shared_ptr<Material> matPentagons = nullptr);
shared_ptr<Mesh> makeBall(std::string name, shared_ptr<Material> mat, shared_ptr<Material> matPentagons, size_t subdivisionSteps);
shared_ptr<Mesh> makeCubeSphere(std::string name, std::array<shared_ptr<Material>, 6> mats, size_t subdivisions, bool singleTexture, bool isCube);

#endif /* __GENERATEMESH_H__ */
