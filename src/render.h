#ifndef __RENDER_H__
#define __RENDER_H__
#include "triangle.h"
#include "object.h"
#include "color.h"
#include "data.h"
#include "matrix.h"
#include <SFML/Graphics.hpp>
#include <iostream>

using sf::Vector3f, sf::Vector2f;
using std::max;

unsigned long millis(){
    return 0;
}

void render() {
    for (size_t i = 0; i < frameSize.x*frameSize.y; i++)
    {
        framebuffer[i] = Color{0, 0, 0, 1};
        zBuffer[i]=1.0f;
    }

#pragma region // ===== COUNT VERTICES & FACES =====
    //std::cout<<("Count vertices & faces ");
    unsigned long t = millis();

    int total_vertices = 0;
    int total_faces = 0;
    for (size_t i = 0; i < objects.size(); i++) {
        Mesh *mesh = objects[i].mesh;
        total_vertices += mesh->n_vertices;
        total_faces += mesh->n_faces;
    }

    //std::cout<<(millis() - t)<<std::endl;
#pragma endregion

    makePerspectiveProjectionMatrix();
    for (size_t i = 0; i < lights.size(); i++)
    {
       Light &light = lights[i];
       light.normal = rotate(Vector3f{0, -1, 0}, light.direction);
    }

#pragma region // ===== PROJECT VERTICES & BUILD TRIANGLES =====
    //std::cout<<("Project vertices & build triangles ");
    t = millis();

    Triangle triangles[total_faces];
    int triI = 0;
    for (size_t i = 0; i < objects.size(); i++) {
        Object obj = objects[i];
        Mesh *mesh = obj.mesh;
        Projection projectedVertices[mesh->n_vertices];
        float objectTransformMatrix[16];
        makeTransformMatrix(obj.rotation, obj.scale, obj.position, objectTransformMatrix);

        for (size_t j = 0; j < mesh->n_vertices; j++) {
            Vertex vV = mesh->vertices[j];
            float vM[4] = {vV.position.x, vV.position.y, vV.position.z, 1};
            matMul(vM, objectTransformMatrix, vM, 1, 4, 4);

            projectedVertices[j] = perspectiveProject({vM[0], vM[1], vM[2]});
        }

        for (size_t j = 0; j < mesh->n_faces; j++) {
            Face face = mesh->faces[j];
            Projection v1s = projectedVertices[face.v1],
                       v2s = projectedVertices[face.v2],
                       v3s = projectedVertices[face.v3];
            Vector3f normalW = (v3s.worldPos - v1s.worldPos).cross(v2s.worldPos - v1s.worldPos).normalized();
            Vector3f normalS = (v3s.screenPos - v1s.screenPos).cross(v2s.screenPos - v1s.screenPos).normalized();
            if(face.invert)
                normalW *= -1.0f;
            if(reverseAllFaces)
                normalW *= -1.0f;
            // lighting
            Color lighting = ambientLight*ambientLight.a; 
            for (size_t i = 0; i < lights.size(); i++)
            {
                Light &light = lights[i];
                float intensity = max(normalW.dot(light.normal), 0.0f);
                if(intensity==0)continue;
                lighting += light.color * light.color.a * intensity;
            }
            lighting *= face.material->diffuse;

            // clang-format off
            triangles[triI] = Triangle{
                .s1 = v1s,
                .s2 = v2s,
                .s3 = v3s,
                .c1 = lighting,
                .c2 = lighting,
                .c3 = lighting,
                .cull = normalS.z < 0 && backFaceCulling
            };
            // clang-format on
            triI++;
        }
    }

    //std::cout<<(millis() - t)<<std::endl;
#pragma endregion

#pragma region // ===== DRAW TRIANGLES =====
    //std::cout<<("Draw triangles ");
    t = millis();

    for (int i = 0; i < total_faces; i++)
    {
        drawTriangle(frameSize, framebuffer, triangles[i]);
    }

    //std::cout<<(millis() - t)<<std::endl;
#pragma endregion
}

#endif /* __RENDER_H__ */
