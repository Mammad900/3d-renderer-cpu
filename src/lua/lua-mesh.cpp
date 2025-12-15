#include "lua-state.h"
#include "../miscTypes.h"
#include "../object.h"
#include "../generateMesh.h"
#include "../gui.h"

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Warray-bounds"
#elif __clang__
#pragma clang diagnostic ignored "-Warray-bounds"
#endif

void luaMesh() {
        auto VertexConstructor = [](sol::table t) {
        Vertex v{};

        auto parse_vec3 = [&](const char* key, Vec3 def) -> Vec3 {
            sol::object o = t[key];
            if (!o.valid()) return def;

            // If it's already a Vec3 userdata
            if (o.is<Vec3>()) {
                return o.as<Vec3>();
            }

            // If it's a table, try numeric indices first, then x,y,z fields
            if (o.get_type() == sol::type::table) {
                sol::table tt = o;
                double x = 0, y = 0, z = 0;

                x = tt.get_or(1, static_cast<double>(def.x));
                y = tt.get_or(2, static_cast<double>(def.y));
                z = tt.get_or(3, static_cast<double>(def.z));
                return Vec3{static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)};
            }

            // If it's a number (unlikely) or otherwise, fallback
            return def;
        };

        auto parse_vec2 = [&](const char* key, Vector2f def) -> Vector2f {
            sol::object o = t[key];
            if (!o.valid()) return def;

            if (o.is<Vector2f>()) {
                return o.as<Vector2f>();
            }

            if (o.get_type() == sol::type::table) {
                sol::table tt = o;
                double x = 0, y = 0;

                x = tt.get_or(1, static_cast<double>(def.x));
                y = tt.get_or(2, static_cast<double>(def.y));
                return Vector2f{static_cast<float>(x), static_cast<float>(y)};
            }

            return def;
        };

        // defaults come from value-initialized Vertex (or supply your own)
        Vec3 default_pos = v.position;
        Vector2f default_uv = v.uv;
        Vec3 default_nrm = v.normal;

        v.position = parse_vec3("position", default_pos);
        v.uv       = parse_vec2("uv", default_uv);
        v.normal   = parse_vec3("normal", default_nrm);

        return v;
    };
    Lua.new_usertype<Vertex>("Vertex",
        sol::constructors<Vertex()>(),
        "position", &Vertex::position,
        "uv", &Vertex::uv,
        "normal", &Vertex::normal,
        sol::meta_function::construct, sol::overload(
            []() { return Vertex(); },
            VertexConstructor
        )
    );

    auto FaceConstructor = [](sol::table t) {
        Face f{};
        f.v1 = t.get_or("v1", static_cast<uint16_t>(t.get_or(1, 0)));
        f.v2 = t.get_or("v2", static_cast<uint16_t>(t.get_or(2, 0)));
        f.v3 = t.get_or("v3", static_cast<uint16_t>(t.get_or(3, 0)));
        f.material = t.get_or("material", nullptr);
        return f;
    };
    Lua.new_usertype<Face>("Face",
        sol::constructors<Face()>(),
        "v1", &Face::v1,
        "v2", &Face::v2,
        "v3", &Face::v3,
        "material", &Face::material,
        sol::meta_function::construct, sol::overload(
            [](uint16_t a, uint16_t b, uint16_t c, shared_ptr<Material> mat) {
                Face f{};
                f.v1 = a; f.v2 = b; f.v3 = c; f.material = mat;
                return f;
            },
            FaceConstructor
        )
    );

    Lua.new_usertype<Mesh>("Mesh",
        // no default value semantics from Lua; return shared_ptr so Lua can own it
        sol::meta_function::construct, sol::overload(
            []() { return std::make_shared<Mesh>(); },
            [VertexConstructor](sol::table t) -> std::shared_ptr<Mesh> {
                Mesh mesh;

                mesh.label = t.get_or<std::string>("label", "");
                mesh.flatShading = t.get_or("flat_shading", false);

                // vertices: iterate the Lua table and convert each entry to Vertex
                sol::object verts_obj = t["vertices"];
                if (verts_obj.valid() && verts_obj.get_type() == sol::type::table) {
                    sol::table verts = verts_obj;
                    for (auto& pair : verts) {
                        sol::object elem = pair.second;
                        // allow already-constructed Vertex userdata or plain tables
                        if (elem.get_type() == sol::type::userdata) {
                            mesh.vertices.push_back(elem.as<Vertex>());
                        } else if(elem.get_type() == sol::type::table) {
                            sol::table ftable = elem;
                            if (ftable[1].valid() || ftable[2].valid() || ftable[3].valid()) {
                            } else {
                                mesh.vertices.push_back(VertexConstructor(ftable));
                            }
                        }
                    }
                }

                // faces: accept either Face userdata/table or simple array-like {1,2,3}
                sol::object faces_obj = t["faces"];
                if (faces_obj.valid() && faces_obj.get_type() == sol::type::table) {
                    sol::table faces = faces_obj;
                    for (auto& pair : faces) {
                        sol::object elem = pair.second;
                        if (elem.get_type() == sol::type::userdata) {
                            mesh.faces.push_back(elem.as<Face>());
                        } else if (elem.get_type() == sol::type::table) {
                            sol::table ft = elem;
                            Face f;
                            f.v1 = static_cast<uint16_t>(ft.get_or(1, 0));
                            f.v2 = static_cast<uint16_t>(ft.get_or(2, 0));
                            f.v3 = static_cast<uint16_t>(ft.get_or(3, 0));
                            f.material = ft.get_or<shared_ptr<Material>>(4, nullptr);
                            mesh.faces.push_back(f);
                        }
                    }
                }

                if(t.get_or("auto_normals", false)) {
                    bakeMeshNormals(mesh);
                }

                auto meshPtr = std::make_shared<Mesh>(std::move(mesh));
                meshes.emplace_back(meshPtr);
                return meshPtr;
            }
        ),

        // expose fields for read/write from Lua
        "label", &Mesh::label,
        // "vertices", &Mesh::vertices, // vector<Vertex> (sol can handle vectors if Vertex usertype exists) // no it cant
        // "faces", &Mesh::faces,       // vector<Face>
        "flatShading", &Mesh::flatShading,
        "vertex_at", [](shared_ptr<Mesh> &mesh, size_t i) {return &mesh->vertices[i-1];},
        "face_at", [](shared_ptr<Mesh> &mesh, size_t i) {return &mesh->faces[i-1];}
    );

    Lua.new_usertype<MeshComponent>("MeshComponent",
        sol::base_classes, sol::bases<Component>(),
        sol::meta_function::construct, [](shared_ptr<Mesh> mesh) {
            return std::make_shared<MeshComponent>(mesh);
        },
        "mesh", &MeshComponent::mesh,
        "as_component", [](shared_ptr<MeshComponent> &c)-> shared_ptr<Component> { return c; }
    );

    Lua["generate_mesh"] = [](sol::this_state s, sol::table t) {
        sol::state_view lua(s);
        std::string type = t["type"];
        shared_ptr<Mesh> mesh;
        if(type == "sphere") {
            mesh = makeSphere(
                t["material"], 
                t.get_or<std::string>("name", "Sphere"), 
                t.get_or("stacks", 20), 
                t.get_or("sectors", 40), 
                t.get_or("invert_u", false),
                t.get_or("invert_v", false)
            );
        } else if(type == "cylinder") {
            shared_ptr<Material> mat = t["material"];
            bool hasEndCap = t.get_or("end_cap", true);
            bool hasStartCap = t.get_or("start_cap", true);
            mesh = makeCylinder(
                mat,  
                t.get_or<std::string>("name", "cylinder"), 
                t.get_or("sectors", 20), 
                hasEndCap ? t.get_or("end_cap_material", mat) : nullptr,
                hasStartCap ? t.get_or("start_cap_material", mat) : nullptr
            );
        } else if(type == "plane") {
            mesh = createPlane(
                t["material"], 
                t.get_or<std::string>("name", "Plane"), 
                t.get_or("subdivisions_x", 1), 
                t.get_or("subdivisions_y", 1)
            );
        } else if (type == "obj") {
            mesh = loadOBJ(
                get_calling_script_path(lua) / t.get<std::string>("file"), 
                t["material"], 
                t.get_or<std::string>("name", t["file"])
            );
        } else if (type == "stl") {
            mesh = loadSTL(
                get_calling_script_path(lua) / t.get<std::string>("file"), 
                t["material"], 
                t.get_or<std::string>("name", t["file"])
            );
        } else if (type == "regular_icosahedron") {
            mesh = makeRegularIcosahedron(
                t.get_or<std::string>("name", "Regular Icosahedron"), 
                t["material"]
            );
        } else if (type == "ico_sphere") {
            mesh = makeIcoSphere(
                t.get_or<std::string>("name", "IcoSphere"), 
                t["material"],
                t.get_or("subdivisions", 3)
            );
        } else if (type == "dodecahedron") {
            bool pentakis = t.get_or("is_pentakis", false);
            mesh = makeDodecahedron(
                t.get_or<std::string>("name", pentakis ? "Pentakis Dodecahedron" : "Regular Dodecahedron"), 
                t["material"], 
                pentakis
            );
        } else if (type == "truncated_icosahedron") {
            mesh = makeTruncatedIcosahedron(
                t.get_or<std::string>("name", "Truncated Icosahedron"), 
                t["material"]
            );
        } else if (type == "ball") {
            shared_ptr<Material> mat = t.get_or("material", nullptr);
            mesh = makeBall(
                t.get_or<std::string>("name", "Ball"), 
                t.get_or("hexagons_material", mat), 
                t.get_or("pentagons_material", mat), 
                t.get_or("subdivisions", 2)
            );
        } else if (type == "cube_sphere" || type == "cube") {
            std::array<std::shared_ptr<Material>, 6> mats;
            bool singleTexture = false;
            if (t["material"].valid()) {
                auto mat = t["material"].get<std::shared_ptr<Material>>();
                mats.fill(mat);
                singleTexture = true;
            } else if (t["materials"].valid()) {
                sol::table tbl = t["materials"];
                for (int i = 0; i < 6; i++) {
                    mats[i] = tbl[i + 1].get<std::shared_ptr<Material>>();
                }
            }
            mesh = makeCubeSphere(
                t.get_or<std::string>("name", "Cube Sphere"), 
                mats, 
                t.get_or("subdivisions", type == "cube" ? 1 : 10),
                singleTexture,
                type == "cube"
            );
        }
        else mesh = std::make_shared<Mesh>();
        mesh->flatShading = t.get_or("flat_shading", mesh->flatShading);
        meshes.emplace_back(mesh);
        return mesh;
    };
}