#ifndef __ENVIRONMENTMAP_H__
#define __ENVIRONMENTMAP_H__

#include "color.h"
#include "texture.h"
#include "vector3.h"
#include <SFML/System/Vector2.hpp>
#include <cstdlib>
#include <array>

extern std::array<std::tuple<float,float,float,float>, 6> cubeMapFaces;

class EnvironmentMap {
  public:
    virtual Color sample(Vec3 lookVector) = 0;
};

class SolidEnvironmentMap : public EnvironmentMap {
  public:
    Color value;
    SolidEnvironmentMap(Color value) : value(value) {}

    Color sample(Vec3 lookVector) {
        return value;
    }
};

class PanoramaMap : public EnvironmentMap {
  public:
    shared_ptr<Texture<Color>> texture;
    PanoramaMap(shared_ptr<Texture<Color>> texture) : texture(texture) {}

    Color sample(Vec3 lookVector);
};

class AtlasCubeMap : public EnvironmentMap {
  public:
    shared_ptr<Texture<Color>> texture;
    AtlasCubeMap(shared_ptr<Texture<Color>> texture) : texture(texture) {}

    Color sample(Vec3 lookVector);
};

class CubeMap : public EnvironmentMap {
  public:
    std::array<shared_ptr<Texture<Color>>, 6> textures;
    CubeMap(std::array<shared_ptr<Texture<Color>>, 6> textures) : textures(textures) {}

    Color sample(Vec3 L);
};

#endif /* __ENVIRONMENTMAP_H__ */
