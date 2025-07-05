#include "generateMesh.h"

using std::string;
constexpr float phi = 1.618033988749895f;

Mesh *makeRegularIcosahedron(string name, Material *mat) {
    vector<Vertex> vertices = {
        {{0, +1, +phi}}, {{0, -1, +phi}}, {{0, +1, -phi}}, {{0, -1, -phi}},
        {{+1, +phi, 0}}, {{-1, +phi, 0}}, {{+1, -phi, 0}}, {{-1, -phi, 0}},
        {{+phi, 0, +1}}, {{+phi, 0, -1}}, {{-phi, 0, +1}}, {{-phi, 0, -1}},
    };
    // Getting these indices right was a pain
    vector<Face> faces = {{0, 8, 4},  {0, 4, 5}, {0, 5, 10},  {0, 1, 8},
                          {1, 10, 0}, {1, 6, 8}, {1, 7, 6},   {2, 5, 4},
                          {2, 11, 5}, {8, 6, 9}, {7, 10, 11}, {1, 10, 7},
                          {2, 4, 9},  {2, 9, 3}, {11, 10, 5}, {2, 3, 11},
                          {3, 7, 11}, {3, 6, 7}, {3, 9, 6},   {4, 8, 9}};
    for (auto &&v : vertices)
        v.normal = -v.position.normalized();
    for (auto &&f : faces)
        f.material = mat;
    return new Mesh(name, vertices, faces, true);
}

void subdivideMesh(Mesh *mesh) {
    // Outer vector is list of vertices
    // Inner vector is list of adjacent vertices
    // Pair: adjacent vertex index, middle point vertex index
    // Goal: when subdividing two adjacent faces, the shared edge would have a middle vertex created twice. Try to deduplicate them
    vector<vector<std::pair<int, int>>> cache(mesh->vertices.size());

    auto splitEdge = [&](uint16_t i1, uint16_t i2) -> uint16_t {
        uint16_t i_1 = std::min(i1, i2);
        uint16_t i_2 = std::max(i1, i2);
        vector<std::pair<int, int>> &neighbors = cache[i_1];
        auto found = std::find_if(neighbors.begin(), neighbors.end(), [&](const std::pair<int, int> &p) {
            return p.first == i_2;
        });
        if (found == neighbors.end()) { // not found, make it then
            Vertex &v1 = mesh->vertices[i_1];
            Vertex &v2 = mesh->vertices[i_2];
            uint16_t iNew = mesh->vertices.size();
            mesh->vertices.emplace_back(
                0.5f * (v1.position + v2.position),
                0.5f * (v1.uv + v2.uv),
                0.5f * (v1.normal + v2.normal)
            );
            neighbors.emplace_back(i_2, iNew);
            return iNew;
        }
        return found->second;
    };

    size_t n_face = mesh->faces.size(); // Since faces are added inside the loop, size changes. Keep initial size cuz we're only iterating over existing ones
    for (size_t i = 0; i < n_face; i++) {
        Face f = mesh->faces[i];
        uint16_t v1 = splitEdge(f.v2, f.v3);
        uint16_t v2 = splitEdge(f.v1, f.v3);
        uint16_t v3 = splitEdge(f.v1, f.v2);
        mesh->faces.emplace_back(f.v3, v2, v1, f.material);
        mesh->faces.emplace_back(v3, f.v2, v1, f.material);
        mesh->faces.emplace_back(v3, v2, f.v1, f.material);
        Face &f2 = mesh->faces[i]; // f isn't a reference because emplace_back might invalidate it. Get the reference after its safe
        f2.v1 = v1; f2.v2 = v2; f2.v3 = v3; // Original face turns into center face
    }
}

Mesh *makeIcoSphere(string name, Material *mat, size_t subdivisionSteps) {
    Mesh *mesh = makeRegularIcosahedron(name, mat);
    for (size_t i = 0; i < subdivisionSteps; i++) {
        subdivideMesh(mesh);
        for (auto &&v : mesh->vertices)
            v.position = v.position.normalized();        
    }
    for (auto &&v : mesh->vertices)
        v.normal = -v.position;
    mesh->faces.shrink_to_fit();
    mesh->vertices.shrink_to_fit();
    return mesh;
}
