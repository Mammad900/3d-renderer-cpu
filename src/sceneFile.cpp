#include <fstream>
#include <filesystem>
#include "data.h"
#include "phongMaterial.h"
#include "earthMaterial.h"
#include "generateMesh.h"

std::istream& operator>>(std::istream& in, Vector3f& v){
    in >> v.x >> v.y >> v.z;
    return in;
}
std::istream& operator>>(std::istream& in, Vector2f& v){
    in >> v.x >> v.y;
    return in;
}
template <typename T>
void getTexture(Texture<T> *&t, std::istream& in, std::filesystem::path &referrer) {
    if(t != nullptr) {
        delete t;
        t = nullptr;
    }
    std::string type;
    T c;
    in >> type >> c;
    if(type == "color") {
        t = new SolidTexture<T>(c);
    }
    else if(type == "texture") {
        std::string path;
        in >> path;
        std::cout << "Loading texture " << path; std::cout.flush();
        sf::Image img(referrer.parent_path() / path);
        std::cout << "."; std::cout.flush();
        ImageTexture<T> *res =  new ImageTexture<T>(img, c);
        std::cout << "." << std::endl;
        t = res;
    } else {
        std::cerr << "Invalid texture type " << type << std::endl;
    }
}
void getNormalMap(std::istream& in, std::filesystem::path &referrer, PhongMaterialProps& mat) {
    std::string filePath;
    int POM;
    float strength;
    in >> filePath >> strength >> POM;
    std::cout << "Loading normal map " << filePath; std::cout.flush();
    sf::Image img(referrer.parent_path() / filePath);
    std::cout << "."; std::cout.flush();
    mat.normalMap = new ImageTexture<Vector3f>(img, {strength, strength, 1});
    if (POM != -1) {
        mat.POM = POM;
        std::cout << "."; std::cout.flush();
        mat.displacementMap = new ImageTexture<float>(img, 1);
    }
    std::cout << "." << std::endl;
}
Material* findMaterial(std::string& name, Scene *scene) {
    for (auto &&mat : scene->materials)
        if(mat->name == name)
            return mat;
    std::cerr << "Could not find material " << name << std::endl;
    return nullptr;
}
Mesh* findMesh(std::string& name, Scene *scene) {
    for (auto &&mesh : scene->meshes)
        if(mesh->label == name)
            return mesh;
    std::cerr << "Could not find mesh " << name << std::endl;
    return nullptr;
}
Scene* findScene(std::string& name) {
    for (auto &&scene : scenes)
        if(scene->name == name)
            return scene;
    std::cerr << "Could not find scene " << name << std::endl;
    return nullptr;
}

void parseSceneFile(std::filesystem::path path, Scene *editingScene) {
    std::ifstream in(path);
    if (!in) {
        std::cerr << "Failed to open scene file.\n";
        return;
    }
    std::string word;
    while (in >> word) {
        // Skip comments
        if (word == "#") {
            while (in >> word && word != "#");
            continue;
        }

        if (word == "import") {
            std::filesystem::path path2;
            in >> path2;
            parseSceneFile(path.parent_path() / path2, editingScene);
        }
        else if (word == "scene") {
            std::string verb, name;
            in >> verb >> name;
            if(verb == "new")
                scenes.push_back(new Scene{.name = name});
            else if(verb == "edit")
                editingScene = findScene(name);
            else if(verb == "render")
                scene = findScene(name);
        }
        else if (word == "cam") {
            in >> word;
            if (word == "pos") in >> editingScene->cam;
            else if (word == "rot") {
                in >> editingScene->camRotation;
                editingScene->camRotation *= M_PIf / 180.0f;
            } else {
                std::cerr << "Invalid cam setting " << word << std::endl;
            }
        } else if (word == "nearFar") {
            in >> editingScene->nearClip >> editingScene->farClip;
        } else if (word == "fov") {
            in >> editingScene->fov;
        } else if (word == "set") {
            in >> word;
            if (word == "renderMode") in >> editingScene->renderMode;
            else if (word == "backFaceCulling") { int x; in >> x; editingScene->backFaceCulling = x; }
            else if (word == "reverseAllFaces") { int x; in >> x; editingScene->reverseAllFaces = x; }
            else if (word == "fullBright") { int x; in >> x; editingScene->fullBright = x; }
            else if (word == "wireFrame") { int x; in >> x; editingScene->wireFrame = x; }
            else if (word == "whitePoint") { in >> editingScene->whitePoint; } 
            else {
                std::cerr << "Invalid setting " << word << std::endl;
            }
        } else if (word == "fog") {
            in >> editingScene->fogColor;
        } else if (word == "ambientLight") {
            in >> editingScene->ambientLight;
        } else if (word == "new") {
            in >> word;
            if (word == "material") {
                std::string name, type;
                in >> name >> type;

                if(type == "phong") {
                    PhongMaterialProps mat{};
                    MaterialFlags flags = MaterialFlags::None;
                    std::string key;
                    while (in >> key && key != "end") {
                        if (key == "#") { while (in >> key && key != "#"); continue; }

                        else if (key == "diffuse") {
                            getTexture(mat.diffuse, in, path);
                        } else if (key == "specular") {
                            getTexture(mat.specular, in, path);
                        } else if (key == "tint") {
                            getTexture(mat.tint, in, path);
                        } else if (key == "emissive") {
                            getTexture(mat.emissive, in, path);
                        } else if (key == "normalMap") {
                            getNormalMap(in, path, mat);
                        } else if (key == "transparent") {
                            flags = static_cast<MaterialFlags>(flags | MaterialFlags::Transparent);
                        } else if (key == "doubleSided") {
                            flags = static_cast<MaterialFlags>(flags | MaterialFlags::DoubleSided);
                        } else {
                            std::cerr << "Invalid material property " << key << std::endl;
                        }
                    }
                    editingScene->materials.push_back(new PhongMaterial(mat, name, flags));
                }
                else if(type == "earth") {
                    EarthMaterial *mat = new EarthMaterial(name);
                    std::string key;
                    while (in >> key && key != "end") {
                        if (key == "#") { while (in >> key && key != "#"); continue; }
                        
                        if(key == "terrainDiffuse") {
                            getTexture(mat->terrainMat->mat.diffuse, in ,path);
                        } else if(key == "oceanDiffuse") {
                            getTexture(mat->oceanMat->mat.diffuse, in ,path);
                        } else if(key == "oceanSpecular") {
                            getTexture(mat->oceanMat->mat.specular, in ,path);
                        } else if(key == "oceanMask") {
                            getTexture(mat->oceanMask, in ,path);
                        } else if(key == "cityLights") {
                            getTexture(mat->terrainMat->mat.emissive, in ,path);
                        } else if(key == "cloudDiffuse") {
                            getTexture(mat->cloudMat->mat.diffuse, in ,path);
                        } else if(key == "cloud") {
                            getTexture(mat->cloudTexture, in ,path);
                        } else if(key == "normalMap") {
                            getNormalMap(in, path, mat->terrainMat->mat);
                        } else {
                            std::cerr << "Invalid material property " << key << std::endl;
                        }
                    }
                    editingScene->materials.push_back(mat);
                } else {
                    std::cerr << "Invalid material type " << type << std::endl;
                }
            } else if (word == "mesh") {
                std::string name, type;
                in >> name >> type;
                if (type == "obj") {
                    std::string matName, path;
                    in >> matName >> path;
                    Mesh* mesh = loadOBJ(path, findMaterial(matName, editingScene), name);
                    editingScene->meshes.push_back(mesh);
                } else if (type == "sphere") {
                    int stacks, sectors;
                    std::string matName;
                    in >> stacks >> sectors >> matName;
                    Mesh* mesh = createSphere(findMaterial(matName, editingScene), name, stacks, sectors);
                    editingScene->meshes.push_back(mesh);
                } else if (type == "plane") {
                    uint16_t subDivX, subDivY;
                    std::string matName;
                    in >> subDivX >> subDivY >> matName;
                    Mesh* mesh = createPlane(findMaterial(matName, editingScene), name, subDivX, subDivY);
                    editingScene->meshes.push_back(mesh);
                } else if (type == "custom") {
                    std::string key;
                    std::vector<Vertex> vertices;
                    std::vector<Face> faces;
                    Material *currentMat;
                    while (in >> key && key != "end") {
                        if (key == "#") { while (in >> key && key != "#"); continue; }
                        
                        if(key == "v") {
                            Vector3f pos;
                            Vector2f uv;
                            uint16_t i;
                            in >> i >> pos >> uv;
                            if(i != vertices.size()){
                                std::cerr << "Invalid vertex index " << i << ", expected " << vertices.size() << std::endl;
                            }
                            vertices.push_back(Vertex{.position = pos, .uv = uv});
                        }
                        else if(key == "f") {
                            uint16_t i1, i2, i3;
                            in >> i1 >> i2 >> i3;
                            faces.push_back(Face{.v1=i1, .v2 = i2, .v3 = i3, .material = currentMat});
                        }
                        else if(key == "m") {
                            std::string matName;
                            in >> matName;
                            currentMat = findMaterial(matName, editingScene);
                        } else {
                            std::cerr << "Invalid custom mesh entry " << key << std::endl;
                        }
                    }
                    editingScene->meshes.push_back(createMesh(faces, vertices, name));
                } else {
                    std::cerr << "Invalid mesh type " << type << std::endl;
                }
            } else if (word == "object") {
                Object *obj = new Object();
                obj->scene = editingScene;
                in >> obj->position >> obj->scale >> obj->rotation;
                obj->rotation *= M_PIf / 180.0f;
                std::string key;
                while (in >> key && key != "end") {
                    if (key == "#") { while (in >> key && key != "#"); continue; }

                    if(key == "mesh") {
                        std::string meshName;
                        in >> meshName;
                        obj->components.push_back(new MeshComponent(obj, findMesh(meshName, editingScene)));
                    }
                    else if(key == "light") {
                        std::string type;
                        Color color;
                        in >> type >> color;

                        Light *light;
                        if (type == "directional") {
                            light = new DirectionalLight(obj, color);
                        }
                        else if (type == "point") {
                            light = new PointLight(obj, color);
                        }
                        else if (type == "spot") {
                            float spreadA, spreadB;
                            in >> spreadA >> spreadB;
                            spreadA *= M_PIf / 180.0f;
                            spreadB *= M_PIf / 180.0f;
                            light = new SpotLight(obj, color, spreadA, spreadB);
                        }
                        else {
                            std::cerr << "Invalid light type " << type << std::endl;
                        }
                        obj->components.push_back(light);
                    }
                    else {
                        std::cerr << "Invalid component type " << key << std::endl;
                    }
                }
                editingScene->objects.push_back(obj);
            } else {
                std::cerr << "Invalid command " << word << std::endl;
            }
        }
    }
    in.close();
}
