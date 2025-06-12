#include <fstream>
#include <iostream>
#include <filesystem>
#include "data.h"
#include "phongMaterial.h"
#include "earthMaterial.h"
#include "generateMesh.h"
#include "textureFiltering.h"
#include "sceneFile.h"

using std::istream, std::cout, std::cerr, std::endl, std::string, std::flush;

void parseObject(Scene *editingScene, std::ifstream &in, Object *parent);

istream& operator>>(istream& in, Vector3f& v){
    in >> v.x >> v.y >> v.z;
    return in;
}
istream& operator>>(istream& in, Vector2f& v){
    in >> v.x >> v.y;
    return in;
}
template <typename T>
void getTexture(Texture<T> *&t, istream& in, std::filesystem::path &referrer) {
    if(t != nullptr) {
        delete t;
        t = nullptr;
    }
    string type;
    T c;
    in >> type >> c;
    if(type == "color") {
        t = new SolidTexture<T>(c);
    }
    else if(type == "texture") {
        string path;
        in >> path;
        cout << "Loading texture " << path << flush;
        sf::Image img(referrer.parent_path() / path);
        cout << "." << flush;
        ImageTexture<T> *res =  new ImageTexture<T>(img, c);
        cout << "." << endl;
        t = res;
    } else {
        cerr << "Invalid texture type " << type << endl;
    }
}
void getNormalMap(istream& in, std::filesystem::path &referrer, PhongMaterialProps& mat) {
    string filePath;
    int POM;
    float strength;
    in >> filePath >> strength >> POM;
    cout << "Loading normal map " << filePath << flush;
    sf::Image img(referrer.parent_path() / filePath);
    cout << "." << flush;
    mat.normalMap = new ImageTexture<Vector3f>(img, {strength, strength, 1});
    if (POM != -1) {
        mat.POM = POM;
        cout << "." << flush;
        mat.displacementMap = new ImageTexture<float>(img, 1);
    }
    cout << "." << endl;
}
Material* findMaterial(string& name, Scene *scene) {
    for (auto &&mat : scene->materials)
        if(mat->name == name)
            return mat;
    cerr << "Could not find material " << name << endl;
    return nullptr;
}
Mesh* findMesh(string& name, Scene *scene) {
    for (auto &&mesh : scene->meshes)
        if(mesh->label == name)
            return mesh;
    cerr << "Could not find mesh " << name << endl;
    return nullptr;
}
Scene* findScene(string& name) {
    for (auto &&scene : scenes)
        if(scene->name == name)
            return scene;
    cerr << "Could not find scene " << name << endl;
    return nullptr;
}

void parseSceneFile(std::filesystem::path path, Scene *editingScene) {
    std::ifstream in(path);
    if (!in) {
        cerr << "Failed to open scene file.\n";
        return;
    }
    string word;
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
            string verb, name;
            in >> verb >> name;
            if(verb == "new")
                scenes.push_back(new Scene{.name = name});
            else if(verb == "edit")
                editingScene = findScene(name);
            else if(verb == "render")
                scene = findScene(name);
        } else if (word == "set") {
            in >> word;
            if (word == "renderMode") in >> editingScene->renderMode;
            else if (word == "backFaceCulling") { int x; in >> x; editingScene->backFaceCulling = x; }
            else if (word == "reverseAllFaces") { int x; in >> x; editingScene->reverseAllFaces = x; }
            else if (word == "fullBright") { int x; in >> x; editingScene->fullBright = x; }
            else if (word == "wireFrame") { int x; in >> x; editingScene->wireFrame = x; }
            else {
                cerr << "Invalid setting " << word << endl;
            }
        } else if (word == "fog") {
            in >> editingScene->fogColor;
        } else if (word == "ambientLight") {
            in >> editingScene->ambientLight;
        } else if (word == "new") {
            in >> word;
            if (word == "material") {
                string name, type;
                in >> name >> type;

                if(type == "phong") {
                    PhongMaterialProps mat{};
                    MaterialFlags flags = MaterialFlags::None;
                    string key;
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
                            cerr << "Invalid material property " << key << endl;
                        }
                    }
                    editingScene->materials.push_back(new PhongMaterial(mat, name, flags));
                }
                else if(type == "earth") {
                    EarthMaterial *mat = new EarthMaterial(name);
                    string key;
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
                            cerr << "Invalid material property " << key << endl;
                        }
                    }
                    editingScene->materials.push_back(mat);
                } else {
                    cerr << "Invalid material type " << type << endl;
                }
            } else if (word == "mesh") {
                string name, type;
                in >> name >> type;
                if (type == "obj") {
                    string matName, path;
                    in >> matName >> path;
                    Mesh* mesh = loadOBJ(path, findMaterial(matName, editingScene), name);
                    editingScene->meshes.push_back(mesh);
                } else if (type == "sphere") {
                    int stacks, sectors;
                    string matName;
                    in >> stacks >> sectors >> matName;
                    Mesh* mesh = createSphere(findMaterial(matName, editingScene), name, stacks, sectors);
                    editingScene->meshes.push_back(mesh);
                } else if (type == "plane") {
                    uint16_t subDivX, subDivY;
                    string matName;
                    in >> subDivX >> subDivY >> matName;
                    Mesh* mesh = createPlane(findMaterial(matName, editingScene), name, subDivX, subDivY);
                    editingScene->meshes.push_back(mesh);
                } else if (type == "custom") {
                    string key;
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
                                cerr << "Invalid vertex index " << i << ", expected " << vertices.size() << endl;
                            }
                            vertices.push_back(Vertex{.position = pos, .uv = uv});
                        }
                        else if(key == "f") {
                            uint16_t i1, i2, i3;
                            in >> i1 >> i2 >> i3;
                            faces.push_back(Face{.v1=i1, .v2 = i2, .v3 = i3, .material = currentMat});
                        }
                        else if(key == "m") {
                            string matName;
                            in >> matName;
                            currentMat = findMaterial(matName, editingScene);
                        } else {
                            cerr << "Invalid custom mesh entry " << key << endl;
                        }
                    }
                    editingScene->meshes.push_back(createMesh(faces, vertices, name));
                } else {
                    cerr << "Invalid mesh type " << type << endl;
                }
            } else if (word == "object") {
                parseObject(editingScene, in, nullptr);
            } else {
                cerr << "Invalid command " << word << endl;
            }
        }
    }
    in.close();
}

void parseObject(Scene* editingScene, std::ifstream &in, Object *parent) {
    Object *obj = new Object();
    obj->scene = editingScene;
    obj->parent = parent;
    if(parent)
        parent->children.push_back(obj);
    else
        editingScene->objects.push_back(obj);
    in >> obj->name >> obj->position >> obj->scale >> obj->rotation;
    obj->rotation *= M_PIf / 180.0f;
    string key;
    while (in >> key && key != "end") {
        if (key == "#") { while (in >> key && key != "#"); continue; }

        if(key == "object") {
            parseObject(editingScene, in, obj);
        } 
        else if(key == "mesh") {
            string meshName;
            in >> meshName;
            obj->components.push_back(new MeshComponent(obj, findMesh(meshName, editingScene)));
        }
        else if(key == "light") {
            string type;
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
                cerr << "Invalid light type " << type << endl;
            }
            obj->components.push_back(light);
        }
        else if(key == "camera") {
            Camera *cam = new Camera(obj);

            while (in >> key && key != "end") {
                if (key == "#") { while (in >> key && key != "#"); continue; }

                if(key == "fov") {
                    in >> cam->fov;
                } else if(key == "nearFar") {
                    in >> cam->nearClip >> cam->farClip;
                } else if(key == "whitePoint") {
                    in >> cam->whitePoint;
                }
            }
            editingScene->camera = cam;
            obj->components.push_back(cam);
        }
        else {
            cerr << "Invalid component type " << key << endl;
        }
    }
}
