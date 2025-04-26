#ifndef __TRIANGLE_H__
#define __TRIANGLE_H__
#include "color.h"
#include "project.h"
#include "textureFiltering.h"
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Image.hpp>
#include <iostream>

using sf::Vector2f, sf::Vector2u, sf::Vector2i;
using std::swap, std::max;

struct Triangle {
    Projection s1, s2, s3;
    Vector2f uv1, uv2, uv3;
    Material *mat;
    bool cull;
};

Vector2f v3to2(Vector3f in) {
    return Vector2f{in.x, in.y};
}

Vector3f v2reflect(Vector3f in, Vector3f normal) {
    return in - normal * in.dot(normal) * 2.0f;
}

size_t frameBufferIndex(Vector2i pos) {
    return pos.x + frameSize.x * pos.y;
}

void plotVertex(Color* frame, Vector2f pos, float depth) {
    depth = std::clamp(depth, 0.0f, 1.0f);
    Color heatColor = Color{depth, 0, 1.0f - depth, 1.0f};
    int cx = static_cast<int>(pos.x);
    int cy = static_cast<int>(pos.y);

    for (int dx = -10; dx <= 10; ++dx) {
        for (int dy = -10; dy <= 10; ++dy) {
            Vector2i p = {cx + dx, cy + dy};
            if (p.x >= 0 && p.y >= 0 && p.x < (int)frameSize.x && p.y < (int)frameSize.y) {
                frame[frameBufferIndex(p)] = heatColor;
            }
        }
    }
}void drawLine(Vector2f from, Vector2f to) {
    Color color = Color{0, 0, 0, 1};

    int x0 = std::round(from.x);
    int y0 = std::round(from.y);
    int x1 = std::round(to.x);
    int y1 = std::round(to.y);

    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        if (x0 >= 0 && y0 >= 0 && x0 < (int)frameSize.x && y0 < (int)frameSize.y)
            framebuffer[frameBufferIndex({x0, y0})] = color;

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


void drawTriangle(Color *frame, Triangle tri) {
    if(tri.cull && backFaceCulling && !(tri.mat->flags & (MaterialFlags::Transparent | MaterialFlags::DoubleSided)))
        return;

    if(
        (tri.s1.screenPos.x < -1 && tri.s2.screenPos.x < -1 && tri.s3.screenPos.x < -1 )||
        (tri.s1.screenPos.x >  1 && tri.s2.screenPos.x >  1 && tri.s3.screenPos.x >  1 )||
        (tri.s1.screenPos.y < -1 && tri.s2.screenPos.y < -1 && tri.s3.screenPos.y < -1 )||
        (tri.s1.screenPos.y >  1 && tri.s2.screenPos.y >  1 && tri.s3.screenPos.y >  1 )||
        (tri.s1.screenPos.z <  nearClip && tri.s2.screenPos.z <  nearClip && tri.s3.screenPos.z <  nearClip )||
        (tri.s1.screenPos.z >  farClip && tri.s2.screenPos.z >  farClip && tri.s3.screenPos.z >  farClip )
    )
        return;

    Vector2f a = (v3to2(tri.s1.screenPos) + Vector2f{1, 1}).componentWiseMul(Vector2f{frameSize.x / 2.0f, frameSize.y / 2.0f}),
             b = (v3to2(tri.s2.screenPos) + Vector2f{1, 1}).componentWiseMul(Vector2f{frameSize.x / 2.0f, frameSize.y / 2.0f}),
             c = (v3to2(tri.s3.screenPos) + Vector2f{1, 1}).componentWiseMul(Vector2f{frameSize.x / 2.0f, frameSize.y / 2.0f});

    // plotVertex(frameSize, frame, a, tri.s1.screenPos.z);
    // plotVertex(frameSize, frame, b, tri.s2.screenPos.z);
    // plotVertex(frameSize, frame, c, tri.s3.screenPos.z);

    if(wireFrame) {
        drawLine(a, b);
        drawLine(c, b);
        drawLine(a, c);
    }

    Vector2f s1 = b - a, s2 = c - a;
    float areaOfTriangle =
        abs(s1.cross(s2)); // Two times the area of the triangle

    Vector3f tangent{}, bitangent{};
    if (tri.mat->needsTBN) {
        Vector3f edge1 = tri.s2.worldPos - tri.s1.worldPos;
        Vector3f edge2 = tri.s3.worldPos - tri.s1.worldPos; 
        Vector2f deltaUV1 = tri.uv2 - tri.uv1;
        Vector2f deltaUV2 = tri.uv3 - tri.uv1; 

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        tangent = Vector3f{
            f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
            f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
            f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z),
        }.normalized();
        bitangent = Vector3f{
            f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x),
            f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y),
            f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z),
        }.normalized();
    }

    auto &&pixel = [&](Vector2i p) -> void
    {
        if(p.x<0 || p.y<0 || p.x>=(int)frameSize.x || p.y>=(int)frameSize.y)
            return;
        size_t index = frameBufferIndex(p);
        if(index > frameSize.x*frameSize.y)
            return;

        Vector2f pp = {(float)p.x + 0.5f, (float)p.y + 0.5f};
        float C1 = -(b-pp).cross(c-pp) / areaOfTriangle;
        float C2 = -(c-pp).cross(a-pp) / areaOfTriangle;
        if(tri.cull) {
            C1 *= -1;
            C2 *= -1;
        }
        float C3 = 1.0f - C1 - C2;
        if(C1<0 || C2 < 0 || C3 < 0)
            return;
        C1 /= tri.s1.screenPos.z;
        C2 /= tri.s2.screenPos.z;
        C3 /= tri.s3.screenPos.z;
        float denom = 1 / (C1 + C2 + C3);

        #define INTERPOLATE_TRI(A,B,C) ((C1*(A) + C2*(B) + C3*(C))*denom)
        float z = INTERPOLATE_TRI(tri.s1.screenPos.z, tri.s2.screenPos.z, tri.s3.screenPos.z);
        Vector3f normal= INTERPOLATE_TRI(tri.s1.normal, tri.s2.normal, tri.s3.normal).normalized();
        Vector3f worldPos= INTERPOLATE_TRI(tri.s1.worldPos, tri.s2.worldPos, tri.s3.worldPos);
        Vector2f uv= INTERPOLATE_TRI(tri.uv1, tri.uv2, tri.uv3);
        #undef INTERPOLATE_TRI

        Color matDiffuse = tri.mat->getBaseColor(uv, {0,0});

        if(matDiffuse.a < 0.5f)
            return;
        if (zBuffer[index] < z || z<0)
            return;
        if(!((tri.mat->flags & MaterialFlags::Transparent)))
            zBuffer[index] = z;

        if(fullBright) {
            frame[index] = matDiffuse;
            return;
        }

        Fragment f{
            .position = worldPos,
            .normal = normal,
            .tangent = tangent,
            .bitangent = bitangent,
            .uv = uv,
            .baseColor = matDiffuse,
            .mat = tri.mat,
            .isBackFace = tri.cull,
        };

        frame[index] = tri.mat->shade(f, frame[index]);

    };

    float minY =(std::min({a.y, b.y, c.y}));
    float maxY =(std::max({a.y, b.y, c.y}));
    float minX =(std::min({a.x, b.x, c.x}));
    float maxX =(std::max({a.x, b.x, c.x}));

    for (int y = minY; y < maxY; y++)
        for (int x = minX; x < maxX; x++)
            pixel({x,y});
}

#endif /* __TRIANGLE_H__ */
