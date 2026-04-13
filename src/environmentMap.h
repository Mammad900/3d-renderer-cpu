#ifndef __ENVIRONMENTMAP_H__
#define __ENVIRONMENTMAP_H__

#include "color.h"
#include "material.h"
#include "texture.h"
#include "vector3.h"
#include "aabb.h"
#include <SFML/System/Vector2.hpp>
#include <array>

extern std::array<std::tuple<float,float,float,float>, 6> cubeMapFaces;

class EnvironmentMap {
  public:
    virtual Color sample(Vec3 lookVector, Vec3 origin={0,0,0}, bool canWriteDepth = false) = 0;
};

class SolidEnvironmentMap : public EnvironmentMap {
  public:
    Color value;
    SolidEnvironmentMap(Color value) : value(value) {}

    Color sample(Vec3 lookVector, Vec3 origin={0,0,0}, bool canWriteDepth = false) {
        return value;
    }
};

class PanoramaMap : public EnvironmentMap {
  public:
    shared_ptr<Texture<Color>> texture;
    PanoramaMap(shared_ptr<Texture<Color>> texture) : texture(texture) {}

    Color sample(Vec3 lookVector, Vec3 origin={0,0,0}, bool canWriteDepth = false);
};

class AtlasCubeMap : public EnvironmentMap {
  public:
    shared_ptr<Texture<Color>> texture;
    AtlasCubeMap(shared_ptr<Texture<Color>> texture) : texture(texture) {}

    Color sample(Vec3 lookVector, Vec3 origin={0,0,0}, bool canWriteDepth = false);
};

class CubeMap : public EnvironmentMap {
  public:
    std::array<shared_ptr<Texture<Color>>, 6> textures;
    CubeMap(std::array<shared_ptr<Texture<Color>>, 6> textures) : textures(textures) {}

    Color sample(Vec3 L, Vec3 origin={0,0,0}, bool canWriteDepth = false);
};

class InfiniteFloor : public EnvironmentMap {
  public:
    shared_ptr<Texture<Color>> texture;
    shared_ptr<EnvironmentMap> fallback;
    float scale;
    InfiniteFloor(shared_ptr<Texture<Color>> texture, shared_ptr<EnvironmentMap> fallback, float scale) 
        : texture(texture), fallback(fallback), scale(scale) {} 

    Color sample(Vec3 L, Vec3 origin={0,0,0}, bool canWriteDepth = false);
};

class ShadedInfiniteFloor : public EnvironmentMap {
  public:
    shared_ptr<Material> material;
    shared_ptr<EnvironmentMap> fallback;
    float scale;
    ShadedInfiniteFloor(shared_ptr<Material> material, shared_ptr<EnvironmentMap> fallback, float scale)
        : material(material), fallback(fallback), scale(scale) {}

    Color sample(Vec3 L, Vec3 origin={0,0,0}, bool canWriteDepth = false);
};

class CubicRoom : public EnvironmentMap {
  public:
    AABB boundingBox;
    shared_ptr<EnvironmentMap> fallback;
    std::array<shared_ptr<Texture<Color>>, 6> textures;
    
    CubicRoom(AABB boundingBox, shared_ptr<EnvironmentMap> fallback, std::array<shared_ptr<Texture<Color>>, 6> textures)
        : boundingBox(boundingBox), fallback(fallback), textures(textures) {}

    Color sample(Vec3 L, Vec3 origin={0,0,0}, bool canWriteDepth = false);
};

#endif /* __ENVIRONMENTMAP_H__ */
