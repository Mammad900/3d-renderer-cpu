#ifndef __GUI_H__
#define __GUI_H__

#include "imgui.h" // necessary for ImGui::*, imgui-SFML.h doesn't include imgui.h
#include "imgui-SFML.h" // for ImGui::SFML::* functions and SFML-specific overloads
#include "math.h"
#include <SFML/Graphics.hpp>

void guiUpdate(sf::RenderWindow &window, sf::Clock &deltaClock)
{
    while (const auto event = window.pollEvent())
    {
        ImGui::SFML::ProcessEvent(window, *event);

        if (event->is<sf::Event::Closed>())
        {
            window.close();
        }
    }

    ImGui::SFML::Update(window, deltaClock.restart());

    ImGui::ShowDemoWindow();

    ImGui::Begin("Options");
    ImGui::SliderFloat("FOV", &fov, 30, 150);
    ImGui::SliderFloat3("Camera rotation", (float *)&camRotation, -M_PI, M_PI);
    ImGui::DragFloat3("Camera position", (float *)&cam, 0.2f);
    ImGui::RadioButton("Frame buffer", &renderMode, 0);
    ImGui::RadioButton("Z buffer", &renderMode, 1);
    ImGui::Checkbox("Back-face culling", &backFaceCulling);
    ImGui::Checkbox("Reverse all faces", &reverseAllFaces);
    ImGui::End();

    ImGui::Begin("Objects");
    for (size_t i = 0; i < objects.size(); i++)
    {
        ImGui::PushID(i);
        if(ImGui::TreeNode("Cube")) {
            Object &obj = objects[i];
            ImGui::SliderFloat3("Rotation", (float *)&obj.rotation, -M_PI, M_PI);
            ImGui::DragFloat3("Position", (float *)&obj.position, 0.2f);
            ImGui::DragFloat3("Scale", (float *)&obj.scale, 0.1f);
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
    ImGui::End();

    ImGui::Begin("Lights");
    ImGui::ColorEdit4("Ambient lighting", (float*)&ambientLight, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR);
    for (size_t i = 0; i < lights.size(); i++)
    {
        ImGui::PushID(i);
        if(ImGui::TreeNode("Light")) {
            Light &light = lights[i];
            ImGui::SliderFloat3("Direction", (float *)&light.direction, -M_PI, M_PI);
            ImGui::ColorEdit4("Color", (float*)&light.color, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR);
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
    ImGui::End();

    ImGui::Begin("Materials");
    for (size_t i = 0; i < materials.size(); i++)
    {
        ImGui::PushID(i);
        if(ImGui::TreeNode("Material")) {
            Material *mat = materials[i];
            ImGui::ColorEdit4("Diffuse", (float*)&mat->diffuse, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR);
            ImGui::ColorEdit4("Specular", (float*)&mat->specular, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR);
            ImGui::DragFloat("Shininess", &mat->shinyness);
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
    ImGui::End();

    ImGui::Begin("Meshes");
    for (size_t i = 0; i < meshes.size(); i++)
    {
        ImGui::PushID(i);
        Mesh *mesh = meshes[i];
        if(ImGui::TreeNode(mesh->label.c_str())) {
            if(ImGui::TreeNode("Vertices")) {
                for (uint16_t j = 0; j < mesh->n_vertices; j++)
                {
                    ImGui::PushID(j);
                    Vertex &v = mesh->vertices[j];
                    ImGui::DragFloat3("Position", &v.position.x, 0.2f);
                    ImGui::DragFloat2("UV", &v.uv.x, 0.2f);
                    ImGui::PopID();
                }
                ImGui::TreePop();
            }
            if(ImGui::TreeNode("Faces")) {
                for (uint16_t j = 0; j < mesh->n_faces; j++)
                {
                    ImGui::PushID(j);
                    Face &f = mesh->faces[j];
                    ImGui::Checkbox("Invert", &f.invert);
                    ImGui::PopID();
                }
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
    ImGui::End();

    // ImGui::ColorEdit4("Color", (float*)&objects[0].mesh->faces[0].c1, ImGuiColorEditFlags_Float);

    window.clear();
    ImGui::SFML::Render(window);
    window.display();
}

#endif /* __GUI_H__ */
