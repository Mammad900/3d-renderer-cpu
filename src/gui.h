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

void guiUpdate(sf::RenderWindow &window, sf::Clock &deltaClock, Scene *editingScene)
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
    ImGui::InputFloat("Near", &editingScene->nearClip);
    ImGui::InputFloat("Far", &editingScene->farClip);
    ImGui::SliderFloat("FOV", &editingScene->fov, 10, 150);
    ImGui::SliderFloat3("Camera rotation", (float *)&editingScene->camRotation, -M_PI, M_PI);
    ImGui::DragFloat3("Camera position", (float *)&editingScene->cam, 0.2f);
    ImGui::RadioButton("Frame buffer", &editingScene->renderMode, 0);
    ImGui::RadioButton("Z buffer", &editingScene->renderMode, 1);
    ImGui::Checkbox("Back-face culling", &editingScene->backFaceCulling);
    ImGui::Checkbox("Reverse all faces", &editingScene->reverseAllFaces);
    ImGui::Checkbox("Full-bright mode", &editingScene->fullBright);
    ImGui::Checkbox("Show wireframe mesh", &editingScene->wireFrame);
    ImGui::Checkbox("Orbit", &editingScene->orbit);
    ImGui::Text("Texture filtering:");
    ImGui::RadioButton("None", &editingScene->textureFilteringMode, 0);
    ImGui::RadioButton("Bilinear", &editingScene->textureFilteringMode, 1);
    ImGui::RadioButton("Trilinear", &editingScene->textureFilteringMode, 2);
    ImGui::SliderFloat("White point", (float *)&editingScene->whitePoint, 0, 5);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::DragScalarN("Frame size", ImGuiDataType_U32, &frameSizeTemp, 2);
    if(ImGui::Button("Set frame size")) {
        changeFrameSize(frameSizeTemp);
    }
    ImGui::End();

    if(ImGui::Begin("Objects")) {
        for (size_t i = 0; i < editingScene->objects.size(); i++) {
            ImGui::PushID(i);
            if(ImGui::TreeNode("Cube")) {
                Object &obj = editingScene->objects[i];
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
                editingScene->objects.push_back(Object{
                    .mesh = selectedMesh,
                    .scale={1,1,1}
                });
            }
            ImGui::TreePop();
        }
    }
    ImGui::End();

    ImGui::Begin("Lights");
    ImGui::ColorEdit4("Ambient lighting", (float*)&editingScene->ambientLight, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR);
    ImGui::ColorEdit4("Fog", (float*)&editingScene->fogColor, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR);
    for (size_t i = 0; i < editingScene->lights.size(); i++)
    {
        ImGui::PushID(i);
        if(ImGui::TreeNode("Light")) {
            Light &light = editingScene->lights[i];
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
        editingScene->lights.push_back(Light{.rotation = {0, 0, 0}, .color = {1, 1, 1, 1}});
    }
    if(ImGui::Button("New point light")) {
        editingScene->lights.push_back(Light{.direction={10,0,0}, .color = {1, 1, 1, 1}, .isPointLight=true});
    }
    ImGui::End();

    if(ImGui::Begin("Materials")) {
        for (size_t i = 0; i < editingScene->materials.size(); i++)
        {
            ImGui::PushID(i);
            Material *mat = editingScene->materials[i];
            if(ImGui::TreeNode(mat->name.c_str())) {
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
        for (size_t i = 0; i < editingScene->meshes.size(); i++)
        {
            ImGui::PushID(i);
            Mesh *mesh = editingScene->meshes[i];
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
                Mesh *m = loadOBJ(objFilePath, selectedMaterial, std::filesystem::path(objFilePath).filename());
                editingScene->meshes.push_back(m);
            }
            ImGui::TreePop();
        }
    }
    ImGui::End();

    if(ImGui::Begin("Scenes")) {
        for (size_t i = 0; i < scenes.size(); i++)
        {
            ImGui::PushID(i);
            Scene *s = scenes[i];
            if(ImGui::RadioButton(s->name.c_str(), s == scene))
                scene = s;
            ImGui::PopID();
        }
    }
    ImGui::End();

    window.clear();
    ImGui::SFML::Render(window);
    window.display();
}

#endif /* __GUI_H__ */
