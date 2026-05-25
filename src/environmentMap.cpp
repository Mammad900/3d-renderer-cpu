#include "environmentMap.h"
#include "data.h"
#include "fog.h"
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

Color PanoramaMap::sample(Vec3 lookVector, Vec3, bool) {
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

Color CubeMap::sample(Vec3 L, Vec3, bool) {
    auto [uv, n] = getCubeMapUV(L);
    return textures[n]->sample(uv, {0,0}, {0,0});
}

Color AtlasCubeMap::sample(Vec3 L, Vec3, bool) {
    auto [uv, n] = getCubeMapUV(L);
    offsets offset = cubeMapFaces[n];
    uv = {
        uv.x * std::get<0>(offset) + std::get<2>(offset), 
        uv.y * std::get<1>(offset) + std::get<3>(offset), 
    };
    return texture->sample(uv, {0,0}, {0,0});
}

Color InfiniteFloor::sample(Vec3 L, Vec3 O, bool canWriteDepth) {
    if(O.y <= 0 || L.y > 0) return fallback->sample(L,O,canWriteDepth);
    float t = -O.y / L.y;
    if(t <= 0) return fallback->sample(L,O,canWriteDepth);
    Vec3 pos = (O + L * t) / scale;
    Vector2f uv { (-pos.x) - floor(-pos.x), pos.z - floor(pos.z)};
    float duv = scale * 0.05f / (L.y * (currentWindow->frame->size.x));
    Color res = texture->sample(uv, {duv,duv}, {duv,duv});
    if(currentWindow->scene->volume)
        res = sampleFog(pos, O, res, *currentWindow->scene, currentWindow->scene->volume);
    return res;
}

Color ShadedInfiniteFloor::sample (Vec3 L, Vec3 O, bool canWriteDepth) {
    if(O.y <= 0 || L.y > 0) return fallback->sample(L,O,canWriteDepth);
    float t = -O.y / L.y;
    if(t <= 0) return fallback->sample(L,O,canWriteDepth);
    Vec3 pos = (O + L * t);
    Vec3 pos2 = pos / scale;
    Vector2f uv { (-pos2.x) - floor(-pos2.x), pos2.z - floor(pos2.z)};
    float duv = scale * 0.05f / (L.y * (currentWindow->frame->size.x));
    Vector2f duv2 {duv,duv};
    
    Face face{0,0,0, material};
    Fragment f{
        .worldPos= pos,
        .normal= {0,-1,0},
        .viewDir = -L,
        .uv = uv,
        .dUVdx= duv2,
        .dUVdy= duv2,
        .baseColor = material->getBaseColor(uv, duv2, duv2),
        .face= &face,
        .isBackFace= false,
    };
    Color res = material->shade(f, Color{}, *currentWindow->scene);
    if(currentWindow->scene->volume)
        res = sampleFog(pos, O, res, *currentWindow->scene, currentWindow->scene->volume);
    return res;
}

Color CubicRoom::sample(Vec3 L, Vec3 O, bool canWriteDepth) {
    int hitAxis = -1;
    bool hitSide = 0;

    float tmin = -INFINITY;
    float tmax =  INFINITY;

    for (int i=0; i<3; ++i) {
        float o = O[i];
        float l = L[i];
        float minB = boundingBox.min[i];
        float maxB = boundingBox.max[i];

        if (fabs(l) < 1e-6f) {
            if (o < minB || o > maxB)
                return fallback->sample(L,O,canWriteDepth);
            continue;
        }

        float t1 = (minB - o) / l;
        float t2 = (maxB - o) / l;
        if (t1 > t2) std::swap(t1, t2);

        if (t1 > tmin) tmin   = t1;
        if (t2 < tmax) {
            tmax = t2;
            hitAxis = i;
            hitSide = l < 0;
        }
    }

    if (tmin > tmax || tmax <= 0)
        return fallback->sample(L,O,canWriteDepth);

    Vec3 pos   = O + L * tmax;
    // Vec3 normal(0,0,0);
    // normal[axis] = side;

    /* ---------- UVs ----------
       Determine the two in‑plane indices:
         axis 0 (X) → uIdx=1(Y), vIdx=2(Z)
         axis 1 (Y) → uIdx=0(X), vIdx=2(Z)
         axis 2 (Z) → uIdx=0(X), vIdx=1(Y)
    */

    // if(pos.x == 1) {hitAxis = 0; hitSide= true;}
    // if(pos.x == -1) {hitAxis = 0; hitSide= true;}

    int uIdx, vIdx;
    if      (hitAxis == 0) { uIdx = 1; vIdx = 2; }
    else if (hitAxis == 1) { uIdx = 0; vIdx = 2; }
    else                   { uIdx = 0; vIdx = 1; }

    float u = (pos[uIdx] - boundingBox.min[uIdx]) / (boundingBox.max[uIdx] - boundingBox.min[uIdx]);
    float v = (pos[vIdx] - boundingBox.min[vIdx]) / (boundingBox.max[vIdx] - boundingBox.min[vIdx]);

    int texId = 0;
    Color i{1,1,1};
    if(hitAxis == 0 && !hitSide) {texId = 0;std::swap(u,v);v = 1-v;u = 1-u;}
    else if(hitAxis == 1 && !hitSide) {texId = 1;i={0,1,0};}
    else if(hitAxis == 2 && !hitSide) {texId = 2;v = 1-v;}
    else if(hitAxis == 0 && hitSide) {texId = 3;std::swap(u,v);v = 1-v;}
    else if(hitAxis == 1 && hitSide) {texId = 4;i={1,0,1};}
    else if(hitAxis == 2 && hitSide) {texId = 5;v = 1-v;u = 1-u;}

    return textures[texId]->sample({u,v}, {0,0}, {0,0});
}