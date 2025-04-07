#ifndef __TRIANGLE_H__
#define __TRIANGLE_H__
#include "color.h"
#include "project.h"
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Image.hpp>
#include <iostream>

using sf::Vector2f, sf::Vector2u, sf::Vector2i;
using std::swap, std::max;

struct Triangle {
    Projection s1, s2, s3;
    Material *mat;
    bool cull;
};

Vector2f v3to2(Vector3f in) {
    return Vector2f{in.x, in.y};
}

Vector3f v2reflect(Vector3f in, Vector3f normal) {
    return in - normal * in.dot(normal) * 2.0f;
}

size_t frameBufferIndex(Vector2u frameSize, Vector2i pos) {
    return pos.x + frameSize.x * pos.y;
}

void plotVertex(Vector2u frameSize, Color* frame, Vector2f pos, float depth) {
    // std::cout << depth << std::endl;
    depth = std::clamp(depth, 0.0f, 1.0f);
    Color heatColor = Color{depth, 0, 1.0f - depth, 1.0f};
    int cx = static_cast<int>(pos.x);
    int cy = static_cast<int>(pos.y);

    for (int dx = -10; dx <= 10; ++dx) {
        for (int dy = -10; dy <= 10; ++dy) {
            Vector2i p = {cx + dx, cy + dy};
            if (p.x >= 0 && p.y >= 0 && p.x < (int)frameSize.x && p.y < (int)frameSize.y) {
                frame[frameBufferIndex(frameSize, p)] = heatColor;
            }
        }
    }
}

void drawTriangle(Vector2u frameSize, Color *frame, Triangle tri) {
    if(tri.cull)
        return;
    
    if(
        (tri.s1.screenPos.x < -1 && tri.s2.screenPos.x < -1 && tri.s3.screenPos.x < -1 )||
        (tri.s1.screenPos.x >  1 && tri.s2.screenPos.x >  1 && tri.s3.screenPos.x >  1 )||
        (tri.s1.screenPos.y < -1 && tri.s2.screenPos.y < -1 && tri.s3.screenPos.y < -1 )||
        (tri.s1.screenPos.y >  1 && tri.s2.screenPos.y >  1 && tri.s3.screenPos.y >  1 )||
        (tri.s1.screenPos.z <  0 && tri.s2.screenPos.z <  0 && tri.s3.screenPos.z <  0 )||
        (tri.s1.screenPos.z >  1 && tri.s2.screenPos.z >  1 && tri.s3.screenPos.z >  1 )
    )
        return;

    Vector2f a = (v3to2(tri.s1.screenPos) + Vector2f{1, 1}).componentWiseMul(Vector2f{frameSize.x / 2.0f, frameSize.y / 2.0f}),
             b = (v3to2(tri.s2.screenPos) + Vector2f{1, 1}).componentWiseMul(Vector2f{frameSize.x / 2.0f, frameSize.y / 2.0f}),
             c = (v3to2(tri.s3.screenPos) + Vector2f{1, 1}).componentWiseMul(Vector2f{frameSize.x / 2.0f, frameSize.y / 2.0f});

    // plotVertex(frameSize, frame, a, tri.s1.screenPos.z);
    // plotVertex(frameSize, frame, b, tri.s2.screenPos.z);
    // plotVertex(frameSize, frame, c, tri.s3.screenPos.z);
    Vector2f s1 = b - a, s2 = c - a;
    float areaOfTriangle =
        abs(s1.cross(s2)); // Two times the area of the triangle

    auto &&pixel = [&](Vector2i p) -> void
    {
        if(p.x<0 || p.y<0 || p.x>=(int)frameSize.x || p.y>=(int)frameSize.y)
            return;
        size_t index = frameBufferIndex(frameSize, p);
        if(index > frameSize.x*frameSize.y)
            return;

        Vector2f pp = {(float)p.x, (float)p.y};
        float C1 = (abs((b-pp).cross(c-pp)) / areaOfTriangle);
        float C2 = (abs((c-pp).cross(a-pp)) / areaOfTriangle);
        float C3 = (1.0f - C1 - C2);
        C1 /= tri.s1.w;
        C2 /= tri.s2.w;
        C3 /= tri.s3.w;
        float denom = 1 / (C1 + C2 + C3);

        #define INTERPOLATE_TRI(A,B,C) ((C1*(A) + C2*(B) + C3*(C))*denom)

        float z = INTERPOLATE_TRI(tri.s1.screenPos.z, tri.s2.screenPos.z, tri.s3.screenPos.z);
        if (zBuffer[index] < z || z<0)
            return;
        zBuffer[index] = z;
        Vector3f normal= INTERPOLATE_TRI(tri.s1.normal, tri.s2.normal, tri.s3.normal).normalized();
        Vector3f worldPos= INTERPOLATE_TRI(tri.s1.worldPos, tri.s2.worldPos, tri.s3.worldPos);

        // =========== LIGHTING ===========

        if(!fullBright){
            // Ambient
            Color diffuse = ambientLight * ambientLight.a, specular={0,0,0,1};

            // Diffuse
            for (size_t i = 0; i < lights.size(); i++)
            {
                Light &light = lights[i];
                if(tri.mat->diffuse.a > 0) {
                    float diffuseIntensity = max(normal.dot(light.normal), 0.0f);
                    diffuse += light.color * light.color.a * diffuseIntensity;
                }
                if(tri.mat->specular.a > 0) {
                    float specularIntensity = pow(max(-camDirection.dot(v2reflect(light.normal, normal)), 0.0f), tri.mat->shinyness);
                    specular += light.color * light.color.a * specularIntensity;
                }
            }
            
            Color lighting = frame[index] = 
                diffuse * tri.mat->diffuse * tri.mat->diffuse.a +
                specular * tri.mat->specular * tri.mat->specular.a;

            maximumColor = max(maximumColor, lighting.luminance());
        } else {
            frame[index] = tri.mat->diffuse;
        }
    };

    std::array<Vector2f, 3> v = {a, b, c};
    // Sort vertices by Y (c>b>a)
    std::sort(v.begin(), v.end(), [](const Vector2f &a, const Vector2f &b) -> bool
              { return a.y < b.y; });
    Vector2f BA = v[1] - v[0];
    Vector2f CB = v[2] - v[1];
    Vector2f CA = v[2] - v[0];

    for (int i = 0; i < BA.y; i++) {
        float x1 = std::round(i * BA.x / BA.y + v[0].x);
        float x2 = std::round(i * CA.x / CA.y + v[0].x);
        if (x1 > x2)
            swap(x1, x2);
        for (int j = x1; j < x2; j++) {
            pixel({j, i + (int)v[0].y});
        }
    }
    for (int i = 0; i < CB.y; i++) {
        float x1 = std::round(i * CB.x / CB.y + v[1].x);
        float x2 = std::round((i + BA.y) * CA.x / CA.y + v[0].x);
        if (x1 > x2)
            swap(x1, x2);
        for (int j = x1; j < x2; j++) {
            pixel({j, i + (int)v[1].y});
        }
    }
}

#endif /* __TRIANGLE_H__ */
