#include "gui.h"
#include "generateMesh.h"

char objFilePath[500];
Material *selectedMaterial;
Mesh *selectedMesh;

void Timing(Metric<float> &m, const char *name) {
    ImGui::Text("%s: Last %04.1f / Mean %04.1f / Max %04.1f", name, m.last, m.average(), m.maximum);
}

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
    ImGui::InputFloat("Near", &editingScene->camera->nearClip);
    ImGui::InputFloat("Far", &editingScene->camera->farClip);
    ImGui::SliderFloat("FOV", &editingScene->camera->fov, 10, 150);
    ImGui::RadioButton("Frame buffer", &editingScene->renderMode, 0);
    ImGui::RadioButton("Z buffer", &editingScene->renderMode, 1);
    ImGui::Checkbox("Back-face culling", &editingScene->backFaceCulling);
    ImGui::Checkbox("Reverse all faces", &editingScene->reverseAllFaces);
    ImGui::Checkbox("Full-bright mode", &editingScene->fullBright);
    ImGui::Checkbox("Show wireframe mesh", &editingScene->wireFrame);
    ImGui::Checkbox("Orbit", &editingScene->orbit);
    ImGui::Checkbox("God-rays", &editingScene->godRays);
    ImGui::SliderFloat("Sample size", &editingScene->godRaysSampleSize, 0.01, 1, "%.3f", ImGuiSliderFlags_Logarithmic);
    ImGui::Text("Texture filtering:");
    ImGui::RadioButton("Nearest Neighbor", &editingScene->textureFilteringMode, TextureFilteringMode::NearestNeighbor);
    ImGui::RadioButton("Bilinear", &editingScene->textureFilteringMode, TextureFilteringMode::Bilinear);
    ImGui::RadioButton("Trilinear", &editingScene->textureFilteringMode, TextureFilteringMode::Trilinear);
    ImGui::SliderFloat("White point", (float *)&editingScene->camera->whitePoint, 0, 5);
    ImGui::End();

    ImGui::Begin("Performance");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    Timing(timing.windowTime, "Window");
    Timing(timing.updateTime, "Update");
    Timing(timing.renderPrepareTime, "Render prepare");
    Timing(timing.geometryTime, "Geometry");
    Timing(timing.lightingTime, "Lighting");
    Timing(timing.forwardTime, "Forward pass");
    Timing(timing.postProcessTIme, "Post processing");
    ImGui::DragScalarN("Frame size", ImGuiDataType_U32, &frameSizeTemp, 2);
    if(ImGui::Button("Set frame size"))
        changeWindowSize(frameSizeTemp);
    if(ImGui::Checkbox("Use Deferred rendering", &frame->deferred))
        frame->changeSize(frame->size, frame->deferred);
    ImGui::End();

    if(ImGui::Begin("Objects")) {
        for (size_t i = 0; i < editingScene->objects.size(); i++) {
            ImGui::PushID(i);
            editingScene->objects[i]->GUI();
            ImGui::PopID();
        }
        ImGui::Spacing();
        if (ImGui::TreeNode("Create object")) {
            if(selectedMesh == nullptr)
                ImGui::Text("Select a mesh in the meshes window.");
            if(selectedMesh!= nullptr && ImGui::Button("Create")) {
                Object *obj = new Object();
                obj->scene = editingScene;
                obj->components.push_back(new MeshComponent(obj, selectedMesh));
                editingScene->objects.push_back(obj);
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
            Light *light = editingScene->lights[i];
            light->GUI();
            ImGui::TreePop();
        }
        ImGui::PopID();
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
                    for (uint16_t j = 0; j < mesh->vertices.size(); j++)
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
            ImGui::RadioButton(s->name.c_str(), &scene, s);
            ImGui::PopID();
        }
    }
    ImGui::End();

    window.clear();
    ImGui::SFML::Render(window);
    window.display();
}
