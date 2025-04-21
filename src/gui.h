#ifndef __GUI_H__
#define __GUI_H__

#include "imgui.h" // necessary for ImGui::*, imgui-SFML.h doesn't include imgui.h
#include "imgui-SFML.h" // for ImGui::SFML::* functions and SFML-specific overloads
#include "math.h"
#include "loadObj.h"
#include <SFML/Graphics.hpp>

char objFilePath[500];
Material *selectedMaterial;
Mesh *selectedMesh;

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
    ImGui::InputFloat("Near", &nearClip);
    ImGui::InputFloat("Far", &farClip);
    ImGui::SliderFloat("FOV", &fov, 10, 150);
    ImGui::SliderFloat3("Camera rotation", (float *)&camRotation, -M_PI, M_PI);
    ImGui::DragFloat3("Camera position", (float *)&cam, 0.2f);
    ImGui::RadioButton("Frame buffer", &renderMode, 0);
    ImGui::RadioButton("Z buffer", &renderMode, 1);
    ImGui::Checkbox("Back-face culling", &backFaceCulling);
    ImGui::Checkbox("Reverse all faces", &reverseAllFaces);
    ImGui::Checkbox("Full-bright mode", &fullBright);
    ImGui::Checkbox("Show wireframe mesh", &wireFrame);
    ImGui::Checkbox("Gamma Correction", &gammaCorrection);
    ImGui::SliderFloat("White point", (float*)&whitePoint, 0, 5);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::End();

    if(ImGui::Begin("Objects")) {
        for (size_t i = 0; i < objects.size(); i++) {
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
        ImGui::Spacing();
        if (ImGui::TreeNode("Create object")) {
            if(selectedMesh == nullptr)
                ImGui::Text("Select a mesh in the meshes window.");
            if(selectedMesh!= nullptr && ImGui::Button("Create")) {
                objects.push_back(Object{
                    .mesh = selectedMesh,
                    .scale={1,1,1}
                });
            }
            ImGui::TreePop();
        }
    }
    ImGui::End();

    ImGui::Begin("Lights");
    ImGui::ColorEdit4("Ambient lighting", (float*)&ambientLight, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR);
    ImGui::ColorEdit4("Fog", (float*)&fogColor, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR);
    for (size_t i = 0; i < lights.size(); i++)
    {
        ImGui::PushID(i);
        if(ImGui::TreeNode("Light")) {
            Light &light = lights[i];
            if(light.isPointLight)
                ImGui::DragFloat3("Position", (float *)&light.direction);
            else
                ImGui::SliderFloat3("Direction", (float *)&light.rotation, -M_PI, M_PI);
            ImGui::ColorEdit4("Color", (float*)&light.color, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR);
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
    if(ImGui::Button("New light")) {
        lights.push_back(Light{.rotation = {0, 0, 0}, .color = {1, 1, 1, 1}});
    }
    if(ImGui::Button("New point light")) {
        lights.push_back(Light{.direction={10,0,0}, .color = {1, 1, 1, 1}, .isPointLight=true});
    }
    ImGui::End();

    if(ImGui::Begin("Materials")) {
        for (size_t i = 0; i < materials.size(); i++)
        {
            ImGui::PushID(i);
            if(ImGui::TreeNode("Material")) {
                Material *mat = materials[i];
                mat->GUI();
                if (mat != selectedMaterial && ImGui::Button("Select"))
                    selectedMaterial = mat;
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
    }
    ImGui::End();

    if(ImGui::Begin("Meshes")) {
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
                if(mesh != selectedMesh && ImGui::Button("Select"))
                    selectedMesh = mesh;
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        ImGui::Spacing();
        if (ImGui::TreeNode("Load OBJ")) {
            ImGui::Text("OBJ file can only contain vertex and face data (no UV) and must be triangulated.");
            if(selectedMaterial == nullptr)
                ImGui::Text("Select a material in the materials window.");
            ImGui::InputText("Path", objFilePath, 500);
            if(selectedMaterial!= nullptr && ImGui::Button("Load")) {
                Mesh *m = loadOBJ(objFilePath, selectedMaterial);
                meshes.push_back(m);
            }
            ImGui::TreePop();
        }
    }
    ImGui::End();

    // ImGui::ColorEdit4("Color", (float*)&objects[0].mesh->faces[0].c1, ImGuiColorEditFlags_Float);

    window.clear();
    ImGui::SFML::Render(window);
    window.display();
}

#endif /* __GUI_H__ */
