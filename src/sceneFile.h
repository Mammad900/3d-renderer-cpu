#ifndef __SCENEFILE_H__
#define __SCENEFILE_H__
#include <fstream>
#include "data.h"
#include "phongMaterial.h"
#include "earthMaterial.h"

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
                std::string type;
                in >> type;

                if(type == "phong") {
                    PhongMaterialProps mat{};
                    MaterialFlags flags = MaterialFlags::None;
                    std::string key;
                    while (in >> key && key != "end") {
                        if (key == "#") { while (in >> key && key != "#"); continue; }

                        if (key == "diffuseColor")
                            in >> mat.diffuse.color.r >> mat.diffuse.color.g >> mat.diffuse.color.b >> mat.diffuse.color.a;
                        else if (key == "specularColor")
                            in >> mat.specular.color.r >> mat.specular.color.g >> mat.specular.color.b >> mat.specular.color.a;
                        else if (key == "tintColor")
                            in >> mat.tint.color.r >> mat.tint.color.g >> mat.tint.color.b >> mat.tint.color.a;
                        else if (key == "emissiveColor")
                            in >> mat.emissive.color.r >> mat.emissive.color.g >> mat.emissive.color.b >> mat.emissive.color.a;
                        else if (key == "diffuseTexture") {
                            std::string path;
                            in >> path;
                            sf::Image img(path);
                            mat.diffuse.texture = loadColorTexture(img);
                        } else if (key == "specularTexture") {
                            std::string path;
                            in >> path;
                            sf::Image img(path);
                            mat.specular.texture = loadColorTexture(img);
                        } else if (key == "tintTexture") {
                            std::string path;
                            in >> path;
                            sf::Image img(path);
                            mat.tint.texture = loadColorTexture(img);
                        } else if (key == "emissiveTexture") {
                            std::string path;
                            in >> path;
                            sf::Image img(path);
                            mat.emissive.texture = loadColorTexture(img);
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
                    materials.push_back(new PhongMaterial(mat, flags));
                }
                else if(type == "earth") {
                    materials.push_back(new EarthMaterial());
                }
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
                obj.rotation *= M_PIf / 180.0f;
                objects.push_back(obj);
            }
        }
    }
    in.close();
}


#endif /* __SCENEFILE_H__ */
