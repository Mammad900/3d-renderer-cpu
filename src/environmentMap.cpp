#include "environmentMap.h"
#include "vector3.h"
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <utility>

// scale x, scale y, offset x, offset y
using offsets = std::tuple<float,float,float,float>;
std::array<offsets, 6> cubeMapFaces {
    offsets{1/3.0f, 0.5f, 0     , 0   },
    offsets{1/3.0f, 0.5f, 1/3.0f, 0   },
    offsets{1/3.0f, 0.5f, 2/3.0f, 0   },
    offsets{1/3.0f, 0.5f, 0     , 0.5f},
    offsets{1/3.0f, 0.5f, 1/3.0f, 0.5f},
    offsets{1/3.0f, 0.5f, 2/3.0f, 0.5f},
};

Color PanoramaMap::sample(Vec3 lookVector) {
    Vector2f uv {
        0.5f -((atan2f(lookVector.z, lookVector.x) + M_PIf / 2.0f) / (2.0f * M_PIf)),
        0.5f - (asinf(lookVector.y) / M_PIf)
    };
    if(uv.x < 0.0f) uv.x += 1.0f;
    return texture->sample(uv, {0, 0}, {0, 0});
}

std::pair<Vector2f, size_t> getCubeMapUV(Vec3 L) {
    Vector2f uv = {0,0};
    size_t n = 0;

    if(abs(L.y) < L.x && abs(L.z) < L.x) { // +x
        uv = (Vector2f{-L.z, -L.y} * (0.5f/L.x));
        n = 0;
    }
    else if(abs(L.x) < L.y && abs(L.z) < L.y) { // +y
        uv = (Vector2f{L.x, L.z} * (0.5f/L.y));
        n = 1;
    }
    else if(abs(L.x) < L.z && abs(L.y) < L.z) { // +z
        uv = (Vector2f{L.x, -L.y} * (0.5f/L.z));
        n = 2;
    }
    else if(abs(L.y) < -L.x && abs(L.z) < -L.x) { // -x
        uv = (Vector2f{L.z, -L.y} * (-0.5f/L.x));
        n = 3;
    }
    else if(abs(L.x) < -L.y && abs(L.z) < -L.y) { // -y
        uv = (Vector2f{L.x, -L.z} * (-0.5f/L.y));
        n = 4;
    }
    else if(abs(L.x) < -L.z && abs(L.y) < -L.z) { // -z
        uv = (Vector2f{-L.x, -L.y} * (-0.5f/L.z));
        n = 5;
    }
    return {uv + Vector2f{0.5f, 0.5f}, n};
}

Color CubeMap::sample(Vec3 L) {
    auto [uv, n] = getCubeMapUV(L);
    return textures[n]->sample(uv, {0,0}, {0,0});
}

Color AtlasCubeMap::sample(Vec3 L) {
    auto [uv, n] = getCubeMapUV(L);
    offsets offset = cubeMapFaces[n];
    uv = {
        uv.x * std::get<0>(offset) + std::get<2>(offset), 
        uv.y * std::get<1>(offset) + std::get<3>(offset), 
    };
    return texture->sample(uv, {0,0}, {0,0});
}