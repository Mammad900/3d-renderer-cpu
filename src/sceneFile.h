#ifndef __SCENEFILE_H__
#define __SCENEFILE_H__
#include <fstream>
#include <filesystem>
#include "data.h"
#include "phongMaterial.h"
#include "earthMaterial.h"

std::istream& operator>>(std::istream& in, Vector3f& v){
    in >> v.x >> v.y >> v.z;
    return in;
}
std::istream& operator>>(std::istream& in, std::optional<Texture<Color>>& t){
    std::string path;
    in >> path;
    sf::Image img(path);
    t = loadColorTexture(img);
    return in;
}
std::istream& operator>>(std::istream& in, std::optional<Texture<Vector3f>>& t){
    std::string path;
    in >> path;
    sf::Image img(path);
    t = loadVectorTexture(img);
    return in;
}
std::istream& operator>>(std::istream& in, std::optional<Texture<float>>& t){
    std::string path;
    in >> path;
    sf::Image img(path);
    t = loadFloatTexture(img);
    return in;
}
Material* findMaterial(std::string& name) {
    for (auto &&mat : materials)
        if(mat->name == name)
            return mat;
    std::cout << "Could not find material " << name << std::endl;
    return nullptr;
}
Mesh* findMesh(std::string& name) {
    for (auto &&mesh : meshes)
        if(mesh->label == name)
            return mesh;
    std::cout << "Could not find mesh " << name << std::endl;
    return nullptr;
}

void parseSceneFile(std::filesystem::path path) {
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
            parseSceneFile(path.parent_path() / path2);
        }
        else if (word == "cam") {
            in >> word;
            if (word == "pos") in >> cam;
            else if (word == "rot") {
                in >> camRotation;
                camRotation *= M_PIf / 180.0f;
            }
        } else if (word == "nearFar") {
            in >> nearClip >> farClip;
        } else if (word == "fov") {
            in >> fov;
        } else if (word == "set") {
            in >> word;
            if (word == "renderMode") in >> renderMode;
            else if (word == "backFaceCulling") { int x; in >> x; backFaceCulling = x; }
            else if (word == "reverseAllFaces") { int x; in >> x; reverseAllFaces = x; }
            else if (word == "fullBright") { int x; in >> x; fullBright = x; }
            else if (word == "wireFrame") { int x; in >> x; wireFrame = x; }
            else if (word == "whitePoint") { in >> whitePoint; }
        } else if (word == "fog") {
            in >> fogColor;
        } else if (word == "ambientLight") {
            in >> ambientLight;
        } else if (word == "new") {
            in >> word;
            if (word == "light") {
                std::string type;
                in >> type;

                Light light;
                if (type == "directional") {
                    in >> light.rotation;
                    light.rotation *= M_PIf / 180.0f;
                    light.isPointLight = false;
                } else if (type == "point") {
                    in >> light.direction;
                    light.isPointLight = true;
                }
                in >> light.color;
                lights.push_back(light);
            } else if (word == "material") {
                std::string name, type;
                in >> name >> type;

                if(type == "phong") {
                    PhongMaterialProps mat{};
                    MaterialFlags flags = MaterialFlags::None;
                    std::string key;
                    while (in >> key && key != "end") {
                        if (key == "#") { while (in >> key && key != "#"); continue; }

                        if (key == "diffuseColor")
                            in >> mat.diffuse.color;
                        else if (key == "specularColor")
                            in >> mat.specular.color;
                        else if (key == "tintColor")
                            in >> mat.tint.color;
                        else if (key == "emissiveColor")
                            in >> mat.emissive.color;
                        else if (key == "diffuseTexture") {
                            in >> mat.diffuse.texture;
                        } else if (key == "specularTexture") {
                            in >> mat.specular.texture;
                        } else if (key == "tintTexture") {
                            in >> mat.tint.texture;
                        } else if (key == "emissiveTexture") {
                            in >> mat.emissive.texture;
                        } else if (key == "normalMap") {
                            std::string path;
                            int POM;
                            in >> path >> POM;
                            sf::Image img(path);
                            mat.normalMap = loadVectorTexture(img);
                            if(POM != -1) {
                                mat.POM = POM;
                                mat.displacementMap = loadFloatTexture(img);
                            }
                        } else if (key == "transparent") {
                            flags = static_cast<MaterialFlags>(flags | Transparent);
                        } else if (key == "doubleSided") {
                            flags = static_cast<MaterialFlags>(flags | DoubleSided);
                        }
                    }
                    materials.push_back(new PhongMaterial(mat, name, flags));
                }
                else if(type == "earth") {
                    materials.push_back(new EarthMaterial(name));
                }
            } else if (word == "mesh") {
                std::string name, type;
                in >> name >> type;
                if (type == "obj") {
                    std::string matName, path;
                    in >> matName >> path;
                    Mesh* mesh = loadOBJ(path, findMaterial(matName), name);
                    meshes.push_back(mesh);
                } else if (type == "sphere") {
                    int stacks, sectors;
                    std::string matName;
                    in >> stacks >> sectors >> matName;
                    Mesh* mesh = createSphere(findMaterial(matName), name, stacks, sectors);
                    meshes.push_back(mesh);
                } else if (type == "plane") {
                    uint16_t subDivX, subDivY;
                    std::string matName;
                    in >> subDivX >> subDivY >> matName;
                    Mesh* mesh = createPlane(findMaterial(matName), name, subDivX, subDivY);
                    meshes.push_back(mesh);
                }
            } else if (word == "object") {
                std::string meshName;
                in >> meshName;
                Object obj;
                obj.mesh = findMesh(meshName);
                in >> obj.position >> obj.scale >> obj.rotation;
                obj.rotation *= M_PIf / 180.0f;
                objects.push_back(obj);
            }
        }
    }
    in.close();
}


#endif /* __SCENEFILE_H__ */
