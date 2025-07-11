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
                          {0, 10, 1}, {1, 6, 8}, {1, 7, 6},   {2, 5, 4},
                          {2, 11, 5}, {8, 6, 9}, {7, 10, 11}, {1, 10, 7},
                          {2, 4, 9},  {2, 9, 3}, {11, 10, 5}, {2, 3, 11},
                          {3, 7, 11}, {3, 6, 7}, {3, 9, 6},   {4, 8, 9}};
    
    for (auto &&v : vertices)
        v.normal = -v.position.normalized();
    for (auto &&f : faces)
        f.material = mat;
    return new Mesh(name, vertices, faces, true);
}

void subdivideMesh2(Mesh *mesh) {
    // Outer vector is list of vertices
    // Inner vector is list of adjacent vertices
    // Pair: adjacent vertex index, middle point vertex index
    // Goal: when subdividing two adjacent faces, the shared edge would have a middle vertex created twice. Try to deduplicate them
    vector<vector<std::pair<int, int>>> cache(mesh->vertices.size());

    auto splitEdge = [&](uint16_t i1, uint16_t i2) -> uint16_t {
        uint16_t i_1 = std::min(i1, i2); // sort i1/i2
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

void subdivideMesh3(Mesh *mesh) {
    // Outer vector is list of vertices
    // Inner vector is list of adjacent vertices
    // Pair: adjacent vertex index, first middle point vertex index
    // Goal: when subdividing two adjacent faces, the shared edge would have the middle vertices created twice. Try to deduplicate them
    vector<vector<std::pair<int, int>>> cache(mesh->vertices.size());

    auto splitEdge = [&](uint16_t i1, uint16_t i2) -> uint16_t {
        uint16_t i_1 = std::min(i1, i2); // sort i1/i2
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
                (1/3.0f) * (2.0f*v1.position + v2.position),
                (1/3.0f) * (2.0f*v1.uv + v2.uv),
                (2.0f*v1.normal + v2.normal).normalized()
            );
            mesh->vertices.emplace_back(
                (1/3.0f) * (v1.position + 2.0f*v2.position),
                (1/3.0f) * (v1.uv + 2.0f*v2.uv),
                (v1.normal + 2.0f*v2.normal).normalized()
            );
            neighbors.emplace_back(i_2, iNew);
            return iNew;
        }
        return found->second;
    };

    size_t n_face = mesh->faces.size(); // Since faces are added inside the loop, size changes. Keep initial size cuz we're only iterating over existing ones
    for (size_t i = 0; i < n_face; i++) {
        Face f = mesh->faces[i];
        uint16_t v_1 = splitEdge(f.v2, f.v3);
        uint16_t v_2 = splitEdge(f.v1, f.v3);
        uint16_t v_3 = splitEdge(f.v1, f.v2);
        uint16_t v111 = f.v1, v222 = f.v2, v333 = f.v3, 
                 v112, v122, v223, v233, v113, v133, v123;
        if(f.v1 > f.v2) {
            v112 = v_3 + 1; v122 = v_3;
        } else {
            v112 = v_3; v122 = v_3 + 1;
        }
        if(f.v1 > f.v3) {
            v113 = v_2 + 1; v133 = v_2;
        } else {
            v113 = v_2; v133 = v_2 + 1;
        }
        if(f.v2 > f.v3) {
            v223 = v_1 + 1; v233 = v_1;
        } else {
            v223 = v_1; v233 = v_1 + 1;
        }
        Vector3f v123pos = (1 / 3.0f) * (mesh->vertices[f.v1].position +
                                         mesh->vertices[f.v2].position +
                                         mesh->vertices[f.v3].position);
        v123 = mesh->vertices.size();
        mesh->vertices.emplace_back(v123pos, Vector2f{0, 0}, -v123pos.normalized());
        //          v111
        //       v112  v113
        //    v122  v123  v133
        // v222  v223  v233  v333
        mesh->faces.emplace_back(v111, v112, v113, f.material);
        mesh->faces.emplace_back(v113, v123, v133, f.material);
        mesh->faces.emplace_back(v133, v233, v333, f.material);
        mesh->faces.emplace_back(v123, v223, v233, f.material);
        mesh->faces.emplace_back(v122, v222, v223, f.material);
        mesh->faces.emplace_back(v112, v122, v123, f.material);
        mesh->faces.emplace_back(v123, v233, v133, f.material);
        mesh->faces.emplace_back(v122, v223, v123, f.material);
        Face &f2 = mesh->faces[i]; // f isn't a reference because emplace_back might invalidate it. Get the reference after its safe
        f2.v1 = v112; f2.v2 = v123; f2.v3 = v113; // Original face turns into center top face
    }
}

Mesh *makeIcoSphere(string name, Material *mat, size_t subdivisionSteps) {
    Mesh *mesh = makeRegularIcosahedron(name, mat);
    for (size_t i = 0; i < subdivisionSteps; i++) {
        subdivideMesh2(mesh);
        for (auto &&v : mesh->vertices)
            v.position = v.position.normalized();        
    }
    for (auto &&v : mesh->vertices)
        v.normal = -v.position;
    mesh->faces.shrink_to_fit();
    mesh->vertices.shrink_to_fit();
    return mesh;
}

// A dodecahedron is like the inverse of a icosahedron
// ico faces become dod vertices, and ico vertices become dod faces
// A pentakis dodecahedron is made by splitting each pentagonal face into five triangular faces and projecting them to a sphere.
// Since in this engine all faces are triangles, the regular dodecahedron returned is technically a pentakis, its just not projected.
Mesh *makeDodecahedron(string name, Material *mat, bool pentakis) {
    Mesh *icosahedron = makeRegularIcosahedron("", nullptr);

    vector<Vertex> vertices;
    vector<Face> faces;
    vertices.reserve(32); // 20 regular vertices + 12 face midpoints
    faces.reserve(60);    // 12 pentagons * 5 per face

    // Find icosahedron face centers which become dodecahedron vertices
    for (auto &&f : icosahedron->faces) {
        Vector3f pos = (1 / 3.0f) * (icosahedron->vertices[f.v1].position +
                                     icosahedron->vertices[f.v2].position +
                                     icosahedron->vertices[f.v3].position);
        pos = pos.normalized();
        vertices.emplace_back(pos, Vector2f{0, 0}, -pos);
    }

    // Triangulates a pentagonal face.
    auto makeFace = [&](uint16_t v1, uint16_t v2, uint16_t v3, uint16_t v4, uint16_t v5) {
        uint16_t i = vertices.size();
        Vector3f pos = 0.2f * (vertices[v1].position + vertices[v2].position +
                               vertices[v3].position + vertices[v4].position +
                               vertices[v5].position);
        if(pentakis) pos = pos.normalized();
        vertices.emplace_back(pos, Vector2f{0, 0}, -pos);

        faces.emplace_back(v1, i, v2, mat);
        faces.emplace_back(v2, i, v3, mat);
        faces.emplace_back(v3, i, v4, mat);
        faces.emplace_back(v4, i, v5, mat);
        faces.emplace_back(v5, i, v1, mat);
    };

    // I found these indices by rendering a regular icosahedron and finding the faces for each vertex
    // Middle
    makeFace( 8, 14, 10, 16, 15);
    makeFace(14,  2,  4, 11, 10);
    makeFace( 0, 19,  9,  5,  3);
    makeFace(19, 12, 13, 18,  9);

    // Top
    makeFace( 7,  8, 15, 13, 12);
    makeFace( 1,  0,  3,  4,  2);
    makeFace( 7, 12, 19,  0,  1);
    makeFace( 1,  2, 14,  8,  7);

    // Bottom
    makeFace( 3,  5,  6, 11,  4);
    makeFace(15, 16, 17, 18, 13);
    makeFace( 9, 18, 17,  6,  5);
    makeFace(10, 11,  6, 17, 16);

    delete icosahedron;
    return new Mesh(name, vertices, faces, true);
}

Mesh *makeTruncatedIcosahedron(string name, Material *mat, Material *matPentagons) {
    if(!matPentagons)
        matPentagons = mat;
    Mesh *mesh = makeRegularIcosahedron(name, mat);
    subdivideMesh3(mesh);
    for (size_t i = 0; i < 12; i++)
        mesh->vertices[i].position = {0, 0, 0};
    for (auto &&f : mesh->faces) {
        if(f.v1 < 12) mesh->vertices[f.v1].position +=
                mesh->vertices[f.v2].position + mesh->vertices[f.v3].position;
        if(f.v2 < 12) mesh->vertices[f.v2].position +=
                mesh->vertices[f.v1].position + mesh->vertices[f.v3].position;
        if(f.v3 < 12) mesh->vertices[f.v3].position +=
                mesh->vertices[f.v1].position + mesh->vertices[f.v2].position;
        if(f.v1 < 12 || f.v2 < 12 || f.v3 < 12)
            f.material = matPentagons;
    }
    for (size_t i = 0; i < 12; i++)
        mesh->vertices[i].position *= 0.1f;

    return mesh;
}

Mesh *makeBall(string name, Material *mat, Material *matPentagons, int subdivisionSteps) {
    Mesh *mesh = makeTruncatedIcosahedron(name, mat, matPentagons);
    for (size_t i = 0; i < subdivisionSteps; i++) {
        subdivideMesh2(mesh);
        for (auto &&v : mesh->vertices)
            v.position = v.position.normalized();        
    }
    for (auto &&v : mesh->vertices)
        v.normal = -v.position;
    mesh->faces.shrink_to_fit();
    mesh->vertices.shrink_to_fit();
    return mesh;
}
