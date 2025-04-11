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
    // std::cout << depth << std::endl;
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
        if (x0 >= 0 && y0 >= 0 && x0 < frameSize.x && y0 < frameSize.y)
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

    if(wireFrame) {
        drawLine(a, b);
        drawLine(c, b);
        drawLine(a, c);
    }

    Vector2f s1 = b - a, s2 = c - a;
    float areaOfTriangle =
        abs(s1.cross(s2)); // Two times the area of the triangle

    std::optional<std::array<float, 16>> TBN;
    if (tri.mat->normalMap) {
        Vector3f edge1 = tri.s2.worldPos - tri.s1.worldPos;
        Vector3f edge2 = tri.s3.worldPos - tri.s1.worldPos; 
        Vector2f deltaUV1 = tri.uv2 - tri.uv1;
        Vector2f deltaUV2 = tri.uv3 - tri.uv1; 

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        Vector3f T = Vector3f{
            f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
            f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
            f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z),
        }.normalized();
        Vector3f B = Vector3f{
            f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x),
            f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y),
            f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z),
        }.normalized();

        // N is filled in each pixel, because normal is interpolated
        TBN = {
            T.x, B.x, 0,
            T.y, B.y, 0,
            T.z, B.z, 0
        };
    }

    auto &&pixel = [&](Vector2i p) -> void
    {
        if(p.x<0 || p.y<0 || p.x>=(int)frameSize.x || p.y>=(int)frameSize.y)
            return;
        size_t index = frameBufferIndex(p);
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
        Vector3f normal= INTERPOLATE_TRI(tri.s1.normal, tri.s2.normal, tri.s3.normal).normalized();
        Vector3f worldPos= INTERPOLATE_TRI(tri.s1.worldPos, tri.s2.worldPos, tri.s3.worldPos);
        Vector2f uv= INTERPOLATE_TRI(tri.uv1, tri.uv2, tri.uv3);


        Color matDiffuse = tri.mat->diffuseTexture == nullptr
                                ? tri.mat->diffuseColor
                                : textureFilter(tri.mat->diffuseTexture, uv);

        if(tri.mat->diffuseTexture != nullptr && matDiffuse.a < 0.5f)
            return;

        if (zBuffer[index] < z || z<0)
            return;
        zBuffer[index] = z;


        // =========== LIGHTING ===========
        if (!fullBright) {
            Color matSpecular = tri.mat->specularTexture == nullptr
                                ? tri.mat->specularColor
                                : textureFilter(tri.mat->specularTexture, uv);
            float shininess = pow(2.0f, matSpecular.a * 25.5f);
            Color matEmissive = tri.mat->emissiveTexture == nullptr
                                ? tri.mat->emissiveColor
                                : textureFilter(tri.mat->emissiveTexture, uv);

            Color diffuse = ambientLight * ambientLight.a;
            Color specular = {0, 0, 0, 1};

            if(TBN) {
                TBN.value()[2] = normal.x;
                TBN.value()[5] = normal.y;
                TBN.value()[8] = normal.z;
                Color normalSample = (textureFilter(tri.mat->normalMap, uv) * 2.0 - 1.0) * Color{-1, -1, 1, 0};
                float normalNew[3] = {normalSample.r, normalSample.g, normalSample.b};
                matMul(TBN.value().data(), normalNew, normalNew, 3, 3, 1);
                normal = {normalNew[0], normalNew[1], normalNew[2]};
            }

            for (size_t i = 0; i < lights.size(); i++)
            {
                Light &light = lights[i];
                Vector3f direction = light.direction;
                float intensity = light.color.a;
                if(light.isPointLight) {
                    Vector3f d = light.direction - worldPos; //direction is world pos in this case
                    float l2 = d.lengthSquared();
                    direction = d / std::sqrtf(l2);
                    intensity /= l2;
                }
                if (tri.mat->diffuseColor.a > 0) {
                    float diffuseIntensity = max(normal.dot(direction), 0.0f);
                    diffuse += light.color * diffuseIntensity * intensity;
                }
                if(matSpecular.a > 0) {
                    float specularIntensity = pow(max(-camDirection.dot(v2reflect(direction, normal)), 0.0f), shininess);
                    specular += light.color * specularIntensity * intensity;
                }
            }
            
            Color lighting = frame[index] = 
                diffuse * matDiffuse +
                specular * matSpecular +
                matEmissive;

            maximumColor = max(maximumColor, lighting.luminance());
        }
        else {
            frame[index] = matDiffuse;
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
