#include <fstream>
#include <iostream>
#include <filesystem>
#include "sceneFile.h"
#include "generateMesh.h"
#include "textureFiltering.h"
#include "phongMaterial.h"
#include "earthMaterial.h"
#include "pbrMaterial.h"

using std::ostream, std::cout, std::cerr, std::endl, std::flush;

// Define serialization for Vec3
ostream& operator<<(ostream& out, const Vec3& v) {
    out << v.x << " " << v.y << " " << v.z;
    return out;
}
ostream& operator<<(ostream& out, const Vector2f& v) {
    out << v.x << " " << v.y;
    return out;
}
ostream& operator<<(ostream& out, const Vector2u& v) {
    out << v.x << " " << v.y;
    return out;
}
ostream& operator<<(ostream& out, const Color& v) {
    out << v.r << " " << v.g << " " << v.b << " " << v.a;
    return out;
}

template <typename T, typename P>
void serializeBlendTexture(std::ofstream& out, BlendTexture<T,P>* texture, const std::filesystem::path& path, int& textureCounter);

// Serialize textures
template <typename T>
void serializeTexture(std::ofstream& out, Texture<T>* texture, const std::filesystem::path& path, int& textureCounter) {
    if (auto* imageTexture = dynamic_cast<ImageTexture<T>*>(texture)) {
        std::filesystem::path texturePath = path.parent_path() / (std::to_string(textureCounter++) + ".png");
        cout << "Saving texture " << texturePath << flush;
        sf::Image textureImage = imageTexture->saveToImage();
        cout << "." << flush;
        if (!textureImage.saveToFile(texturePath.string()))
            cerr << "Failed to save texture" << texturePath << endl;
        cout << "." << endl;
        out << "texture " << imageTexture->value << " " << texturePath.filename().string() << "\n";
    } 
    else if (auto* solidTexture = dynamic_cast<SolidTexture<T>*>(texture)) {
        out << "color " << solidTexture->value << "\n";
    } 
    else if (auto* swt = dynamic_cast<SineWaveTexture*>(texture)) {
        out << "sineWave " << swt->a << " " << swt->b << " " << swt->c << " "
            << swt->d << " " << swt->e << " " << swt->orientation << "\n";
    }
    else if (auto* blendTexture = dynamic_cast<BlendTexture<T,T>*>(texture)) {
        serializeBlendTexture(out, blendTexture, path, textureCounter);
    }
    else if (auto* blendTexture = dynamic_cast<BlendTexture<T,float>*>(texture)) {
        serializeBlendTexture(out, blendTexture, path, textureCounter);
    }
    else {
        out << "color 0 0 0 0\n"; // Default for unsupported texture types
    }
}

template <typename T, typename P>
void serializeBlendTexture(std::ofstream& out, BlendTexture<T,P>* texture, const std::filesystem::path& path, int& textureCounter) {
    out << "blend ";
    switch (texture->mode) {
    case BlendMode::Multiply:
        if constexpr(std::is_same_v<P, float>)
            out << "multiplyFloat ";
        else
            out << "multiply ";
        break;
    case BlendMode::Add:
        out << "add ";
        break;
    case BlendMode::Subtract:
        out << "subtract ";
        break;
    case BlendMode::AlphaMix:
        out << "alpha ";
        break;
    default:
        break;
    }
    out << "\n        ";
    serializeTexture(out, texture->a, path, textureCounter);
    out << "\n        ";
    serializeTexture(out, texture->b, path, textureCounter);
}

void serializeNormalMap(std::ofstream& out, PhongMaterialProps& mat, const std::filesystem::path& path, int& textureCounter) {
    auto map = dynamic_cast<ImageTexture<Vec3>*>(mat.normalMap.value());
    std::filesystem::path texturePath = path.parent_path() / (std::to_string(textureCounter++) + ".png");
    cout << "Saving normal map " << texturePath << flush;
    sf::Image textureImage = map->saveToImage();
    cout << "." << flush;
    if(!textureImage.saveToFile(texturePath.string()))
        cerr << "Failed to save texture" << texturePath << endl;
    cout << "." << endl;
    out << texturePath.filename().string() << " " << map->value.x << " " 
        << (mat.displacementMap ? int(mat.POM) : -1) << "\n";
}

void serializeMaterialFlags(Material *phongMaterial, std::ofstream &out) {
    if (phongMaterial->flags.transparent) {
        out << "    transparent\n";
    }
    if (phongMaterial->flags.doubleSided) {
        out << "    doubleSided\n";
    }
    if (phongMaterial->flags.alphaCutout) {
        out << "    alphaCutout\n";
    }
}

// Serialize materials
void serializeMaterial(std::ofstream& out, Material* material, const std::filesystem::path& path, int& textureCounter) {
    if (auto* phongMaterial = dynamic_cast<PhongMaterial*>(material)) {
        PhongMaterialProps &mat = phongMaterial->mat;
        out << "new material " << phongMaterial->name << " phong\n";
        if (mat.diffuse) {
            out << "    diffuse ";
            serializeTexture(out, mat.diffuse, path, textureCounter);
        }
        if (mat.specular) {
            out << "    specular ";
            serializeTexture(out, mat.specular, path, textureCounter);
        }
        if (mat.tint) {
            out << "    tint ";
            serializeTexture(out, mat.tint, path, textureCounter);
        }
        if (mat.emissive) {
            out << "    emissive ";
            serializeTexture(out, mat.emissive, path, textureCounter);
        }
        if (mat.normalMap) {
            out << "    normalMap ";
            serializeNormalMap(out, mat, path, textureCounter);
        }
        serializeMaterialFlags(phongMaterial, out);
        out << "end\n\n";
    } else if (auto* earthMaterial = dynamic_cast<EarthMaterial*>(material)) {
        out << "new material " << earthMaterial->name << " earth\n";
        if (earthMaterial->terrainMat->mat.diffuse) {
            out << "    terrainDiffuse ";
            serializeTexture(out, earthMaterial->terrainMat->mat.diffuse, path, textureCounter);
        }
        if (earthMaterial->oceanMat->mat.diffuse) {
            out << "    oceanDiffuse ";
            serializeTexture(out, earthMaterial->oceanMat->mat.diffuse, path, textureCounter);
        }
        if (earthMaterial->oceanMat->mat.specular) {
            out << "    oceanSpecular ";
            serializeTexture(out, earthMaterial->oceanMat->mat.specular, path, textureCounter);
        }
        if (earthMaterial->oceanMask) {
            out << "    oceanMask ";
            serializeTexture(out, earthMaterial->oceanMask, path, textureCounter);
        }
        if (earthMaterial->terrainMat->mat.emissive) {
            out << "    cityLights ";
            serializeTexture(out, earthMaterial->terrainMat->mat.emissive, path, textureCounter);
        }
        if (earthMaterial->cloudMat->mat.diffuse) {
            out << "    cloudDiffuse ";
            serializeTexture(out, earthMaterial->cloudMat->mat.diffuse, path, textureCounter);
        }
        if (earthMaterial->cloudTexture) {
            out << "    cloud ";
            serializeTexture(out, earthMaterial->cloudTexture, path, textureCounter);
        }
        if (earthMaterial->terrainMat->mat.normalMap) {
            out << "    normalMap ";
            serializeNormalMap(out, earthMaterial->terrainMat->mat, path, textureCounter);
        }
        out << "end\n\n";
    } else if (auto* pbrMaterial = dynamic_cast<PBRMaterial*>(material)) {
        out << "new material " << pbrMaterial->name << " pbr\n";
        out << "    albedo ";
        serializeTexture(out, pbrMaterial->albedo, path, textureCounter);
        out << "    metallic ";
        serializeTexture(out, pbrMaterial->metallic, path, textureCounter);
        out << "    roughness ";
        serializeTexture(out, pbrMaterial->roughness, path, textureCounter);
        out << "    ambientOcclusion ";
        serializeTexture(out, pbrMaterial->ambientOcclusion, path, textureCounter);
        serializeMaterialFlags(pbrMaterial, out);
        out << "end\n\n";
    }
    else {
        cerr << "Unsupported material type for serialization." << endl;
    }
}


void serializeObject(std::ofstream& out, Object* object, const std::filesystem::path& path, int& textureCounter, std::string indent) {
    out << indent << (indent.length() ? "object " : "new object ") << object->name << " \n" 
        << indent << "    " << object->position << "\n"
        << indent << "    " << object->scale << "\n" 
        << indent << "    " << (object->rotation * (180.0f / M_PIf)) << "\n";
    std::string indentPrev = indent;
    indent += "    ";

    for (auto&& component : object->components) {
        out << "\n";
        if (auto* meshComponent = dynamic_cast<MeshComponent*>(component)) {
            out << indent << "mesh " << meshComponent->mesh->label << "\n";
        } else if (auto* light = dynamic_cast<Light*>(component)) {
            out << indent << "light ";
            if (auto* directionalLight = dynamic_cast<DirectionalLight*>(light)) {
                out << "directional " << directionalLight->color << "\n";
            } else if (auto* pointLight = dynamic_cast<PointLight*>(light)) {
                out << "point " << pointLight->color << "\n";
            } else if (auto* spotLight = dynamic_cast<SpotLight*>(light)) {
                out << "spot " << spotLight->color << " "
                    << spotLight->spreadInner * (180.0f / M_PIf) << " "
                    << spotLight->spreadOuter * (180.0f / M_PIf) << "\n";
                if (spotLight->shadowMap) {
                    out << "1 " << spotLight->shadowMap->tFrame->size << "\n";
                } else {
                    out << "0\n";
                }
            }
        } else if (auto* camera = dynamic_cast<Camera*>(component)) {
            out << indent << "camera\n";
            out << indent << "    fov " << camera->fov << "\n";
            out << indent << "    nearFar " << camera->nearClip << " " << camera->farClip << "\n";
            out << indent << "    whitePoint " << camera->whitePoint << "\n";
            out << indent << "end\n";
        } else if (auto* rotator = dynamic_cast<RotatorComponent*>(component)) {
            out << indent << "rotator " << rotator->rotatePerSecond * (180.0f / M_PIf) << "\n";
        } else if (auto* keyboardControl = dynamic_cast<KeyboardControlComponent*>(component)) {
            out << indent << "keyboardControl " << keyboardControl->speed << " " << keyboardControl->scaleIsChildZ << "\n";
        }
    }

    for (auto&& child : object->children) {
        out << "\n";
        serializeObject(out, child, path, textureCounter, indent);
    }

    out << indentPrev << "end\n";
}

// Serialize scene
void serializeSceneToFile(Scene* scene, const std::filesystem::path& path) {
    int textureCounter = 0;
    std::ofstream out(path);
    if (!out) {
        cerr << "Failed to open file for writing: " << path << endl;
        return;
    }

    out << "scene new " << scene->name << "\n";
    out << "scene edit " << scene->name << "\n\n";

    // Serialize scene settings
    out << "set renderMode " << scene->renderMode << "\n";
    out << "set backFaceCulling " << scene->backFaceCulling << "\n";
    out << "set reverseAllFaces " << scene->reverseAllFaces << "\n";
    out << "set fullBright " << scene->fullBright << "\n";
    out << "set wireFrame " << scene->wireFrame << "\n";
    out << "set windowSize " << frame->size << "\n\n";
    out << "fog " << scene->fogColor << "\n";

    if (scene->skyBox) {
        out << "skyBox ";
        serializeTexture(out, scene->skyBox, path, textureCounter);
    }

    out << "ambientLight " << scene->ambientLight << "\n\n\n";

    out << "# Materials #\n\n";

    // Serialize materials
    for (auto&& material : scene->materials) {
        serializeMaterial(out, material, path, textureCounter);
    }

    out << "\n# Meshes #\n";

    // Serialize meshes
    for (auto&& mesh : scene->meshes) {
        out << "\n";
        out << "new mesh " << mesh->label << " raw\n";
        if(mesh->flatShading)
            out << "flat\n";
        for (size_t i = 0; i < mesh->vertices.size(); ++i) {
            const auto& v = mesh->vertices[i];
            out << "v " << i << " " << v.position<< " " << v.uv << " " << v.normal << "\n";
        }
        std::vector<Face> faces = mesh->faces;
        auto &&compareMat = [](Face &a, Face &b){ return a.material > b.material; };
        Material *lastMat = nullptr;
        std::sort(faces.begin(), faces.end(), compareMat);
        for (const auto &f : mesh->faces) {
            if(lastMat != f.material)
                out << "m " << f.material->name << "\n";
            lastMat = f.material;
            out << "f " << f.v1 << " " << f.v2 << " " << f.v3 << "\n";
        }
        out << "end\n";
    }

    out << "\n# Objects #\n";

    // Serialize objects
    for (auto&& object : scene->objects) {
        out << "\n";
        serializeObject(out, object, path, textureCounter, "");
    }

    out.close();
}

void serializeEverything(const std::filesystem::path &path) {
    std::filesystem::remove_all(path);
    std::filesystem::create_directory(path);
    std::ofstream out(path / "scene.txt");
    for (auto &&scene : scenes) {
        std::filesystem::create_directory(path / scene->name);
        serializeSceneToFile(scene, path / scene->name / "scene.txt");
        out << "import " << scene->name << "/scene.txt\n";
    }
    out << "\nscene render " << scene->name << "\n";
    out.close();
}
