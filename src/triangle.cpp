#include "triangle.h"
#include "color.h"
#include "textureFiltering.h"
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Image.hpp>
#include <iostream>

using sf::Vector2f, sf::Vector2u, sf::Vector2i;
using std::swap, std::max, std::abs;


Vector2f v3to2(Vector3f in) {
    return Vector2f{in.x, in.y};
}
Vector2f v2abs(Vector2f in) {
    return Vector2f{abs(in.x), abs(in.y)};
}

void plotVertex(RenderTarget* frame, Vector2f pos, float depth) {
    depth = std::clamp(depth, 0.0f, 1.0f);
    Color heatColor = Color{depth, 0, 1.0f - depth, 1.0f};
    int cx = static_cast<int>(pos.x);
    int cy = static_cast<int>(pos.y);

    for (int dx = -10; dx <= 10; ++dx) {
        for (int dy = -10; dy <= 10; ++dy) {
            Vector2i p = {cx + dx, cy + dy};
            if (p.x >= 0 && p.y >= 0 && p.x < (int)frame->size.x && p.y < (int)frame->size.y) {
                frame->framebuffer[p.x + p.y * frame->size.x] = heatColor;
            }
        }
    }
}

void drawLine(Vector2f from, Vector2f to, RenderTarget *frame) {
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
    RenderTarget *frame = camera->tFrame;
    Scene *scene = camera->obj->scene;

    if(
            (camera->shadowMap ? !tri.cull : tri.cull) && // Shadow maps have front face culling
            scene->backFaceCulling &&
            !(tri.mat->flags & (MaterialFlags::Transparent | MaterialFlags::DoubleSided))
    )
        return;

    if(
        (tri.s1.screenPos.x < -1 && tri.s2.screenPos.x < -1 && tri.s3.screenPos.x < -1 )||
        (tri.s1.screenPos.x >  1 && tri.s2.screenPos.x >  1 && tri.s3.screenPos.x >  1 )||
        (tri.s1.screenPos.y < -1 && tri.s2.screenPos.y < -1 && tri.s3.screenPos.y < -1 )||
        (tri.s1.screenPos.y >  1 && tri.s2.screenPos.y >  1 && tri.s3.screenPos.y >  1 )||
        (tri.s1.screenPos.z <  camera->nearClip && tri.s2.screenPos.z <  camera->nearClip && tri.s3.screenPos.z <  camera->nearClip )||
        (tri.s1.screenPos.z >  camera->farClip && tri.s2.screenPos.z >  camera->farClip && tri.s3.screenPos.z >  camera->farClip )
    )
        return;

    Vector2f a = (v3to2(tri.s1.screenPos) + Vector2f{1, 1}).componentWiseMul(Vector2f{frame->size.x / 2.0f, frame->size.y / 2.0f}),
             b = (v3to2(tri.s2.screenPos) + Vector2f{1, 1}).componentWiseMul(Vector2f{frame->size.x / 2.0f, frame->size.y / 2.0f}),
             c = (v3to2(tri.s3.screenPos) + Vector2f{1, 1}).componentWiseMul(Vector2f{frame->size.x / 2.0f, frame->size.y / 2.0f});

    // plotVertex(frameSize, frame, a, tri.s1.screenPos.z);
    // plotVertex(frameSize, frame, b, tri.s2.screenPos.z);
    // plotVertex(frameSize, frame, c, tri.s3.screenPos.z);

    if(scene->wireFrame) {
        drawLine(a, b, frame);
        drawLine(c, b, frame);
        drawLine(a, c, frame);
    }

    float areaOfTriangle = abs((b - a).cross(c - a)); // Two times the area of the triangle

    Vector3f tangent{}, bitangent{};
    if (tri.mat->needsTBN) {
        Vector3f edge1 = tri.s2.worldPos - tri.s1.worldPos;
        Vector3f edge2 = tri.s3.worldPos - tri.s1.worldPos; 
        Vector2f deltaUV1 = tri.uv2 - tri.uv1;
        Vector2f deltaUV2 = tri.uv3 - tri.uv1; 

        float f = 1.0f / deltaUV1.cross(deltaUV2);
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

    auto &&getFragment = [&](Vector2i p) -> Fragment {
        Vector2f pp = {(float)p.x + 0.5f, (float)p.y + 0.5f};
        float C1 = -(b-pp).cross(c-pp) / areaOfTriangle;
        float C2 = -(c-pp).cross(a-pp) / areaOfTriangle;
        if(tri.cull) {
            C1 *= -1;
            C2 *= -1;
        }
        float C3 = 1.0f - C1 - C2;
        bool inside = C1 >= 0 && C2 >= 0 && C3 >= 0;
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
        if(
            f.screenPos.x < 0 || 
            f.screenPos.y < 0 || 
            f.screenPos.x >= (int)frame->size.x || 
            f.screenPos.y >= (int)frame->size.y ||
            !f.inside
        )
            return;
        size_t index = f.screenPos.x + f.screenPos.y * frame->size.x;
        // if(index > frame->size.x * frame->size.y)
        //     return;

        Color baseColor = f.baseColor =
            (frame->deferred && !(tri.mat->flags & MaterialFlags::AlphaCutout))
                ? Color{0, 0, 0, 1}
                : tri.mat->getBaseColor(f.uv, f.dUVdx, f.dUVdy);

        if(baseColor.a < 0.5f)
            return;
        if (frame->zBuffer[index] < f.z || f.z<0)
            return;
        if(!(tri.mat->flags & MaterialFlags::Transparent))
            frame->zBuffer[index] = f.z;

        if(defer) {
            frame->gBuffer[index] = f;
        } else {
            frame->framebuffer[index] = scene->fullBright ?
                baseColor :
                tri.mat->shade(f, frame->framebuffer[index], camera->obj->scene);
        }
    };

    float minY = std::min({a.y, b.y, c.y});
    float maxY = std::max({a.y, b.y, c.y});
    float minX = std::min({a.x, b.x, c.x});
    float maxX = std::max({a.x, b.x, c.x});

    for (int y = minY; y < maxY; y+=2)
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