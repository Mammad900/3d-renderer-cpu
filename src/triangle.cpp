#include "triangle.h"
#include "color.h"
#include "data.h"
#include "textureFiltering.h"
#include "fog.h"
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Image.hpp>
#include <cstdint>
#include <iostream>

using sf::Vector2f, sf::Vector2u, sf::Vector2i;
using std::swap, std::min, std::max, std::abs, std::round;


Vector2f v3to2(Vec3 in) {
    return Vector2f{in.x, in.y};
}
Vector2f v2abs(Vector2f in) {
    return Vector2f{abs(in.x), abs(in.y)};
}

void drawLine(Vector2f from, Vector2f to, RenderTarget *frame) {
    Color color = Color{0, 0, 0, 1};

    int x0 = round(from.x);
    int y0 = round(from.y);
    int x1 = round(to.x);
    int y1 = round(to.y);

    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        if (x0 >= 0 && y0 >= 0 && x0 < (int)frame->size.x && y0 < (int)frame->size.y)
            frame->framebuffer[x0 + y0 * frame->size.x] = color;

        if (x0 == x1 && y0 == y1)
            break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void drawTriangle(Camera *camera, Triangle tri, bool defer) {
    RenderTarget *frame = camera->frame;
    shared_ptr<Scene> scene = camera->obj->scene.lock();
    if(!scene) return;

    if (
            (camera->shadowMap ? !tri.cull : tri.cull) && // Shadow maps have front face culling
            scene->backFaceCulling &&
            !(tri.mat->flags.transparent || tri.mat->flags.doubleSided)
    )
        return;

    if (
        (tri.s1.screenPos.x < -1 && tri.s2.screenPos.x < -1 && tri.s3.screenPos.x < -1) || // Frustum culling left
        (tri.s1.screenPos.x >  1 && tri.s2.screenPos.x >  1 && tri.s3.screenPos.x >  1) || // Frustum culling right
        (tri.s1.screenPos.y < -1 && tri.s2.screenPos.y < -1 && tri.s3.screenPos.y < -1) || // Frustum culling up
        (tri.s1.screenPos.y >  1 && tri.s2.screenPos.y >  1 && tri.s3.screenPos.y >  1) || // Frustum culling down
        (tri.s1.screenPos.z <  camera->nearClip && tri.s2.screenPos.z <  camera->nearClip && tri.s3.screenPos.z <  camera->nearClip) || // Too close
        (tri.s1.screenPos.z >  camera->farClip && tri.s2.screenPos.z >  camera->farClip && tri.s3.screenPos.z >  camera->farClip) || // Too far
        !(tri.s1.screenPos.z > 0 && tri.s2.screenPos.z > 0 && tri.s3.screenPos.z > 0) // negative z
    )
        return;

    Vector2f a = (v3to2(tri.s1.screenPos) + Vector2f{1, 1}).componentWiseMul(Vector2f{frame->size.x / 2.0f, frame->size.y / 2.0f}),
             b = (v3to2(tri.s2.screenPos) + Vector2f{1, 1}).componentWiseMul(Vector2f{frame->size.x / 2.0f, frame->size.y / 2.0f}),
             c = (v3to2(tri.s3.screenPos) + Vector2f{1, 1}).componentWiseMul(Vector2f{frame->size.x / 2.0f, frame->size.y / 2.0f});

    if (scene->wireFrame) {
        drawLine(a, b, frame);
        drawLine(c, b, frame);
        drawLine(a, c, frame);
    }

    float areaOfTriangle = abs((b - a).cross(c - a)); // Two times the area of the triangle

    Vec3 triangleNormal = tri.mesh->flatShading ?
         (tri.s3.worldPos - tri.s1.worldPos).cross(tri.s2.worldPos - tri.s1.worldPos).normalized()
         : Vec3{0,0,0};

    Vec3 tangent{}, bitangent{};
    if (tri.mat->needsTBN) {
        Vec3 edge1 = tri.s2.worldPos - tri.s1.worldPos;
        Vec3 edge2 = tri.s3.worldPos - tri.s1.worldPos; 
        Vector2f deltaUV1 = tri.uv2 - tri.uv1;
        Vector2f deltaUV2 = tri.uv3 - tri.uv1; 

        tangent = Vec3{
            deltaUV2.y * edge1.x - deltaUV1.y * edge2.x,
            deltaUV2.y * edge1.y - deltaUV1.y * edge2.y,
            deltaUV2.y * edge1.z - deltaUV1.y * edge2.z,
        }.normalized();
        bitangent = Vec3{
            -deltaUV2.x * edge1.x + deltaUV1.x * edge2.x,
            -deltaUV2.x * edge1.y + deltaUV1.x * edge2.y,
            -deltaUV2.x * edge1.z + deltaUV1.x * edge2.z,
        }.normalized();
    }

    auto &&getFragment = [&](Vector2i p) -> Fragment {
        Vector2f pp = {(float)p.x + 0.5f, (float)p.y + 0.5f};
        float C1 = -(b-pp).cross(c-pp) / areaOfTriangle;
        float C2 = -(c-pp).cross(a-pp) / areaOfTriangle;
        if (tri.cull) { // If backface, C1 and C2 are negative
            C1 *= -1;
            C2 *= -1;
        }
        float C3 = 1.0f - C1 - C2; // C1+C2+C3=1 so take a shortcut
        bool inside = C1 >= 0 && C2 >= 0 && C3 >= 0;
        C1 /= tri.s1.screenPos.z; // Perspective correction
        C2 /= tri.s2.screenPos.z;
        C3 /= tri.s3.screenPos.z;
        float denom = 1 / (C1 + C2 + C3);

        #define INTERPOLATE_TRI(A,B,C) ((C1*(A) + C2*(B) + C3*(C))*denom)
        float z =           INTERPOLATE_TRI(tri.s1.screenPos.z, tri.s2.screenPos.z, tri.s3.screenPos.z);
        Vec3 normal = tri.mesh->flatShading ? triangleNormal : 
                            INTERPOLATE_TRI(tri.s1.normal, tri.s2.normal, tri.s3.normal).normalized();
        Vec3 worldPos = INTERPOLATE_TRI(tri.s1.worldPos, tri.s2.worldPos, tri.s3.worldPos);
        Vector2f uv =       INTERPOLATE_TRI(tri.uv1, tri.uv2, tri.uv3);
        #undef INTERPOLATE_TRI

        Fragment f{
            .screenPos = p,
            .z = z,
            .worldPos = worldPos,
            .normal = normal,
            .tangent = tangent,
            .bitangent = bitangent,
            .uv = uv,
            .face = tri.face,
            .isBackFace = tri.cull,
            .inside = inside
        };
        return f;
    };
    auto &&postFragment = [&](Fragment &f) -> void {
        if (
            f.screenPos.x < 0 || 
            f.screenPos.y < 0 || 
            f.screenPos.x >= (int)frame->size.x || 
            f.screenPos.y >= (int)frame->size.y ||
            !f.inside
        )
            return;
        size_t index = f.screenPos.x + f.screenPos.y * frame->size.x;

        Color baseColor = f.baseColor =
            (defer && !tri.mat->flags.alphaCutout)
                ? Color{0, 0, 0, 1}
                : tri.mat->getBaseColor(f.uv, f.dUVdx, f.dUVdy);

        if (baseColor.a < 0.5f)
            return;
        if ((frame->zBuffer[index] < f.z) || f.z<0)
            return;
        float previousZ = frame->zBuffer[index];
        if(!(defer && tri.mat->flags.transparent))
            frame->zBuffer[index] = f.z;

        if (defer) {
            if (tri.mat->flags.transparent) {
                uint32_t &currentHead = frame->transparencyHeads[index];
                uint32_t prev = UINT32_MAX;
                uint32_t current = currentHead;
            
                // Traverse the linked list to find the correct insertion point (z descending)
                while (current != UINT32_MAX && frame->transparencyFragments[current].f.z > f.z) {
                    prev = current;
                    current = frame->transparencyFragments[current].next;
                }
            
                // Insert fragment
                frame->transparencyFragments.push_back(FragmentNode{f, current});
                uint32_t newIndex = frame->transparencyFragments.size() - 1;
            
                // Update the linked list pointers
                if (prev == UINT32_MAX) { // inserted at front of list, have to update head
                    currentHead = newIndex;
                } else { // Inserted in the middle or end of the list
                    frame->transparencyFragments[prev].next = newIndex;
                }
            }
            else
                frame->gBuffer[index] = f;
        } else {
            shared_ptr<Volume> volume = f.isBackFace ? tri.mat->volumeFront : tri.mat->volumeBack;
            if(!volume) volume = scene->volume;
            if(volume && tri.mat->flags.transparent) { // Fog behind the fragment
                if(previousZ == INFINITY)
                    previousZ = camera->farClip;
                if(previousZ != INFINITY || (scene->volume && scene->volume->godRays)) {
                    Vec3 previousPixelPos = camera->screenSpaceToWorldSpace(f.screenPos.x, f.screenPos.y, previousZ);
                    frame->framebuffer[index] = sampleFog(previousPixelPos, f.worldPos, frame->framebuffer[index], *scene, volume);
                }
            }
            frame->framebuffer[index] = scene->fullBright ?
                baseColor :
                tri.mat->shade(f, frame->framebuffer[index], *scene);
        }
    };

    float minY = min({a.y, b.y, c.y});
    float maxY = max({a.y, b.y, c.y});
    float minX = min({a.x, b.x, c.x});
    float maxX = max({a.x, b.x, c.x});

    for (int y = minY; y < maxY; y+=2) {
        for (int x = minX; x < maxX; x+=2) {
            Fragment f1 = getFragment({x  ,y  });
            Fragment f2 = getFragment({x+1,y  });
            Fragment f3 = getFragment({x  ,y+1});
            Fragment f4 = getFragment({x+1,y+1});
            f1.dUVdx = f2.uv - f1.uv;
            f2.dUVdx = f2.uv - f1.uv;
            f3.dUVdx = f4.uv - f3.uv;
            f4.dUVdx = f4.uv - f3.uv;
            f1.dUVdy = f3.uv - f1.uv;
            f3.dUVdy = f3.uv - f1.uv;
            f2.dUVdy = f4.uv - f2.uv;
            f4.dUVdy = f4.uv - f2.uv;
            postFragment(f1);
            postFragment(f2);
            postFragment(f3);
            postFragment(f4);
        }
    }
}