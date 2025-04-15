#ifndef __SCENEFILE_H__
#define __SCENEFILE_H__
#include <fstream>
#include "data.h"

void parseSceneFile(std::string path) {
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
            std::string path;
            in >> path;
            parseSceneFile(path);
        }
        else if (word == "cam") {
            in >> word;
            if (word == "pos") in >> cam.x >> cam.y >> cam.z;
            else if (word == "rot") {
                in >> camRotation.x >> camRotation.y >> camRotation.z;
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
            in >> fogColor.r >> fogColor.g >> fogColor.b >> fogColor.a;
        } else if (word == "ambientLight") {
            in >> ambientLight.r >> ambientLight.g >> ambientLight.b >> ambientLight.a;
        } else if (word == "new") {
            in >> word;
            if (word == "light") {
                std::string type;
                in >> type;

                Light light;
                if (type == "directional") {
                    in >> light.rotation.x >> light.rotation.y >> light.rotation.z;
                    light.rotation *= M_PIf / 180.0f;
                    light.isPointLight = false;
                } else if (type == "point") {
                    in >> light.direction.x >> light.direction.y >> light.direction.z;
                    light.isPointLight = true;
                }
                in >> light.color.r >> light.color.g >> light.color.b >> light.color.a;
                lights.push_back(light);
            } else if (word == "material") {
                Material* mat = new Material{};
                std::string key;
                while (in >> key && key != "end") {
                    if (key == "#") { while (in >> key && key != "#"); continue; }

                    if (key == "diffuseColor")
                        in >> mat->diffuseColor.r >> mat->diffuseColor.g >> mat->diffuseColor.b >> mat->diffuseColor.a;
                    else if (key == "specularColor")
                        in >> mat->specularColor.r >> mat->specularColor.g >> mat->specularColor.b >> mat->specularColor.a;
                    else if (key == "tintColor")
                        in >> mat->tintColor.r >> mat->tintColor.g >> mat->tintColor.b >> mat->tintColor.a;
                    else if (key == "emissiveColor")
                        in >> mat->emissiveColor.r >> mat->emissiveColor.g >> mat->emissiveColor.b >> mat->emissiveColor.a;
                    else if (key == "diffuseTexture") {
                        std::string path;
                        in >> path;
                        mat->diffuseTexture = new sf::Image();
                        if (!mat->diffuseTexture->loadFromFile(path))
                            std::cerr << "Failed to load texture: " << path << "\n";
                    } else if (key == "specularTexture") {
                        std::string path;
                        in >> path;
                        mat->specularTexture = new sf::Image();
                        if (!mat->specularTexture->loadFromFile(path))
                            std::cerr << "Failed to load texture: " << path << "\n";
                    } else if (key == "tintTexture") {
                        std::string path;
                        in >> path;
                        mat->tintTexture = new sf::Image();
                        if (!mat->tintTexture->loadFromFile(path))
                            std::cerr << "Failed to load texture: " << path << "\n";
                    } else if (key == "emissiveTexture") {
                        std::string path;
                        in >> path;
                        mat->emissiveTexture = new sf::Image();
                        if (!mat->emissiveTexture->loadFromFile(path))
                            std::cerr << "Failed to load texture: " << path << "\n";
                    } else if (key == "normalMap") {
                        std::string path;
                        in >> path;
                        mat->normalMap = new sf::Image();
                        if (!mat->normalMap->loadFromFile(path))
                            std::cerr << "Failed to load texture: " << path << "\n";
                    } else if (key == "transparent") {
                        mat->flags = static_cast<MaterialFlags>(mat->flags | Transparent);
                    } else if (key == "disableBackfaceCulling") {
                        mat->flags = static_cast<MaterialFlags>(mat->flags | DisableBackfaceCulling);
                    }
                }
                materials.push_back(mat);
            } else if (word == "mesh") {
                std::string type;
                in >> type;
                if (type == "obj") {
                    int matIndex;
                    std::string path;
                    in >> matIndex >> path;
                    Mesh* mesh = loadOBJ(path, materials[matIndex]);
                    meshes.push_back(mesh);
                } else if (type == "sphere") {
                    int stacks, sectors, matIndex;
                    in >> stacks >> sectors >> matIndex;
                    Mesh* mesh = createSphere(stacks, sectors, materials[matIndex]);
                    meshes.push_back(mesh);
                } else if (type == "plane") {
                    int matIndex;
                    uint16_t subDivX, subDivY;
                    in >> subDivX >> subDivY >> matIndex;
                    Mesh* mesh = createPlane(subDivX, subDivY, materials[matIndex]);
                    meshes.push_back(mesh);
                }
            } else if (word == "object") {
                int meshIndex;
                Object obj;
                in >> meshIndex;
                obj.mesh = meshes[meshIndex];
                in >> obj.position.x >> obj.position.y >> obj.position.z;
                in >> obj.scale.x >> obj.scale.y >> obj.scale.z;
                in >> obj.rotation.x >> obj.rotation.y >> obj.rotation.z;
                objects.push_back(obj);
            }
        }
    }
    in.close();
}


#endif /* __SCENEFILE_H__ */
