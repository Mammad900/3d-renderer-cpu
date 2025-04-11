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

void fog();

void render() {
    for (size_t i = 0; i < frameSize.x*frameSize.y; i++)
    {
        framebuffer[i] = Color{0, 0, 0, 1};
        zBuffer[i]=1.0f;
    }

#pragma region // ===== COUNT VERTICES & FACES =====

    int total_vertices = 0;
    int total_faces = 0;
    for (size_t i = 0; i < objects.size(); i++) {
        Mesh *mesh = objects[i].mesh;
        total_vertices += mesh->n_vertices;
        total_faces += mesh->n_faces;
    }

#pragma endregion

    makePerspectiveProjectionMatrix();

    camDirection = rotate({0, 0, 1}, camRotation);

    for (size_t i = 0; i < lights.size(); i++) {
        Light &light = lights[i];
        if(!light.isPointLight)
            light.direction = rotate(Vector3f{0, -1, 0}, light.rotation);
    }

#pragma region // ===== PROJECT VERTICES & BUILD TRIANGLES =====

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
            float normal[4] = {vV.normal.x, vV.normal.y, vV.normal.z, 1};
            matMul(normal, objectTransformMatrix, normal, 1, 4, 4);

            projectedVertices[j] = perspectiveProject({vM[0], vM[1], vM[2]});
            projectedVertices[j].normal = (Vector3f{normal[0], normal[1], normal[2]} - obj.position).normalized(); 
            // ^ The transform we applied to the normal vector also scales and translates it, have to undo those.
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

            triangles[triI] = Triangle{
                .s1 = v1s,
                .s2 = v2s,
                .s3 = v3s,
                .uv1 = mesh->vertices[face.v1].uv,
                .uv2 = mesh->vertices[face.v2].uv,
                .uv3 = mesh->vertices[face.v3].uv,
                .mat = face.material,
                .cull = normalS.z < 0 && backFaceCulling
            };
            triI++;
        }
    }

#pragma endregion

#pragma region // ===== DRAW TRIANGLES =====

    for (int i = 0; i < total_faces; i++)
    {
        drawTriangle(framebuffer, triangles[i]);
    }

#pragma endregion

#pragma region // ===== FOG =====
    if(fogColor.a > 0)
        fog();

#pragma endregion
}

void fog() {

    float tanFOV = tan(fov * M_PI / 360);
    for (int y = 0; y < (int)frameSize.y; y++) {
        for (int x = 0; x < (int)frameSize.x; x++) {
            size_t i = frameBufferIndex({x, y});
            float clipZ = zBuffer[i];
            float worldZ = (nearClip * farClip) /
                           (farClip + nearClip - clipZ * (farClip - nearClip)
                           ); // Reconstruct world space Z from Z buffer
            Vector2f world =
                Vector2f{x / (float)frameSize.x, y / (float)frameSize.y} *
                worldZ * tanFOV; // Reconstruct world space X and Y
            float dist = std::sqrt(
                world.lengthSquared() + worldZ * worldZ
            ); // Account for X and Y, to make it radial vs flat
            float visibility = std::clamp(
                std::powf(0.5f, dist * fogColor.a), 0.0f, 1.0f
            ); // Exponential falloff
            Color result = framebuffer[i] * visibility +
                           fogColor * (1 - visibility); // Lerp
            framebuffer[i] = result;
        }
    }
}

#endif /* __RENDER_H__ */
