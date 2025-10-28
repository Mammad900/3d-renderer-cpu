#include "gui.h"
#include "data.h"
#include "generateMesh.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "material.h"
#include "sceneFile.h"
#include "phongMaterial.h"
#include "lua.h"
#include <iostream>
#include <memory>
#include <string>

char objFilePath[500];
shared_ptr<Material> guiSelectedMaterial;
vector<std::weak_ptr<Material>> materials;
vector<std::weak_ptr<Volume>> volumes;
Mesh *selectedMesh;
GuiMaterialAssignMode guiMaterialAssignMode;
std::string luaReplInput;

void Timing(Metric<float> &m, const char *name) {
    ImGui::Text("%s: Last %04.1f / Mean %04.1f / Max %04.1f", name, m.last, m.average(), m.maximum);
}

void guiUpdate(sf::RenderWindow &window, sf::Clock &deltaClock, shared_ptr<Scene> editingScene)
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

    if(ImGui::Begin("Lua REPL")) {
        ImGui::InputTextMultiline("##", &luaReplInput);
        if(ImGui::Button("Run"))
            luaRun(luaReplInput);
    }
    ImGui::End();

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
    ImGui::Checkbox("Bilinear shadow filtering", &editingScene->bilinearShadowFiltering);
    ImGui::Text("Texture filtering:");
    ImGui::RadioButton("Nearest Neighbor", &editingScene->textureFilteringMode, TextureFilteringMode::NearestNeighbor);
    ImGui::RadioButton("Bilinear", &editingScene->textureFilteringMode, TextureFilteringMode::Bilinear);
    ImGui::RadioButton("Trilinear", &editingScene->textureFilteringMode, TextureFilteringMode::Trilinear);
    ImGui::SliderFloat("White point", (float *)&editingScene->camera->whitePoint, 0, 5);
    ImGui::End();

    ImGui::Begin("Performance");
    ImGui::Checkbox("Render", &timing.render);
    if(ImGui::Button("Render one frame now and save")) {
        editingScene->camera->render();
        sf::Image res = editingScene->camera->getRenderedFrame(0);
        if(!res.saveToFile("render.png"))
            std::cerr << "Failed to save to render.png" << std::endl;
    }
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    Timing(timing.windowTime, "Window");
    Timing(timing.updateTime, "Update");
    Timing(timing.skyBoxTime, "SkyBox");
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
        // ImGui::Spacing();
        // if (ImGui::TreeNode("Create object")) {
        //     if(selectedMesh == nullptr)
        //         ImGui::Text("Select a mesh in the meshes window.");
        //     if(selectedMesh!= nullptr && ImGui::Button("Create")) {
        //         Object *obj = new Object();
        //         obj->scene = editingScene;
        //         obj->components.push_back(std::make_shared<MeshComponent>(obj, selectedMesh));
        //         editingScene->objects.push_back(obj);
        //     }
        //     ImGui::TreePop();
        // }
    }
    ImGui::End();

    if(ImGui::Begin("Lights")) {
        ImGui::ColorEdit4("Ambient lighting", (float*)&editingScene->ambientLight, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR);
        ImGui::DragFloat("Shadow bias", &editingScene->shadowBias, 0.05);
        bool needsCleanup = false;
        for (auto &&volume_w : volumes) {
            if(auto volume = volume_w.lock()) {
                ImGui::PushID(volume.get());
                if(ImGui::TreeNode(("Volume: "+volume->name).c_str())) {
                    ImGui::ColorEdit4("Diffuse", &volume->diffuse.r, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR);
                    ImGui::ColorEdit4("Emissive", &volume->emissive.r, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR);
                    if(ImGui::ColorEdit4("Transmission", &volume->transmission.r, ImGuiColorEditFlags_Float))
                        volume->updateIntensity();
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
            else needsCleanup = true;
        }
        if (needsCleanup) {
            volumes.erase(
                std::remove_if(volumes.begin(), volumes.end(),
                            [](auto &w) { return w.expired(); }),
                volumes.end());
        }

        for (size_t i = 0; i < editingScene->lights.size(); i++) {
            ImGui::PushID(i);
            if(ImGui::TreeNode("Light")) {
                Light *light = editingScene->lights[i];
                light->GUI();
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
    }
    ImGui::End();

    if(ImGui::Begin("Materials")) {
        bool needsCleanup = false;
        static ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
        for (size_t i = 0; i < materials.size(); i++)
        {
            ImGui::PushID(i);
            if(std::shared_ptr<Material> mat = materials[i].lock()){
                ImGuiTreeNodeFlags flags = baseFlags | (guiSelectedMaterial == mat ? ImGuiTreeNodeFlags_Selected : 0);
                bool open = ImGui::TreeNodeEx(mat->name.c_str(), flags);
                if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                    guiSelectedMaterial = mat;
                if(open) {
                    mat->GUI();
                    ImGui::TreePop();
                }
            }
            else needsCleanup = true;
            ImGui::PopID();
        }

        if (needsCleanup) {
            materials.erase(
                std::remove_if(materials.begin(), materials.end(),
                            [](auto &w) { return w.expired(); }),
                materials.end());
        }

        ImGui::Spacing();
        if (ImGui::TreeNodeEx("Material changer", ImGuiTreeNodeFlags_SpanAvailWidth)) {
            ImGui::Text("Enable and click on a face/object to change its material to selected one");
            if(!guiSelectedMaterial) {
                ImGui::Text("Select a material first");
                ImGui::BeginDisabled();
            }
            ImGui::RadioButton("None", &guiMaterialAssignMode, GuiMaterialAssignMode::None);
            ImGui::RadioButton("Face", &guiMaterialAssignMode, GuiMaterialAssignMode::Face);
            ImGui::RadioButton("Mesh", &guiMaterialAssignMode, GuiMaterialAssignMode::Mesh);
            if(!guiSelectedMaterial)
                ImGui::EndDisabled();
            ImGui::TreePop();
        }
    }
    ImGui::End();

    // if(ImGui::Begin("Meshes")) {
    //     for (size_t i = 0; i < editingScene->meshes.size(); i++)
    //     {
    //         ImGui::PushID(i);
    //         Mesh *mesh = editingScene->meshes[i];
    //         if(ImGui::TreeNode(mesh->label.c_str())) {
    //             ImGui::Text("%lu vertices, %lu faces", mesh->vertices.size(), mesh->faces.size());
    //             ImGui::Checkbox("Flat shading", &mesh->flatShading);
    //             if(ImGui::TreeNode("Vertices")) {
    //                 for (uint16_t j = 0; j < mesh->vertices.size(); j++)
    //                 {
    //                     ImGui::PushID(j);
    //                     Vertex &v = mesh->vertices[j];
    //                     ImGui::DragFloat3("Position", &v.position.x, 0.2f);
    //                     ImGui::DragFloat2("UV", &v.uv.x, 0.2f);
    //                     ImGui::PopID();
    //                 }
    //                 ImGui::TreePop();
    //             }
    //             if(ImGui::TreeNode("Faces")) {
    //                 static Face *highlightedFace = nullptr;
    //                 static shared_ptr<Material> highlightedMaterial = nullptr;
    //                 static shared_ptr<Material> highlightMat = std::make_shared<PhongMaterial>(PhongMaterialProps{
    //                     .emissive = std::make_shared<SolidTexture<Color>>(Color{1,1,0,1})
    //                 }, (std::string)"Highlight", MaterialFlags{.doubleSided = true});

    //                 for (uint16_t j = 0; j < mesh->faces.size(); j++) {
    //                     ImGui::PushID(j);
    //                     Face &f = mesh->faces[j];
    //                     uint16_t step = 1;
    //                     std::string label = std::to_string(j);
    //                     ImGui::InputScalarN(label.c_str(), ImGuiDataType_U16, &f.v1, 3, &step);
    //                     if(ImGui::RadioButton("Highlight", &f == highlightedFace)) {
    //                         if(highlightedFace) {
    //                             highlightedFace->material = highlightedMaterial;
    //                         }
    //                         if(&f == highlightedFace)
    //                             highlightedFace = nullptr;
    //                         else {
    //                             highlightedFace = &f;
    //                             highlightedMaterial = f.material;
    //                             f.material = highlightMat;
    //                         }
    //                     }
    //                     ImGui::PopID();
    //                 }
    //                 ImGui::TreePop();
    //             }
    //             if(mesh != selectedMesh && ImGui::Button("Select"))
    //                 selectedMesh = mesh;
    //             ImGui::TreePop();
    //         }
    //         ImGui::PopID();
    //     }
    //     // ImGui::Spacing();
    //     // if (ImGui::TreeNode("Load OBJ")) {
    //     //     ImGui::Text("OBJ file can only contain vertex and face data (no UV) and must be triangulated.");
    //     //     if(guiSelectedMaterial == nullptr)
    //     //         ImGui::Text("Select a material in the materials window.");
    //     //     ImGui::InputText("Path", objFilePath, 500);
    //     //     if(guiSelectedMaterial!= nullptr && ImGui::Button("Load")) {
    //     //         Mesh *m = loadOBJ(objFilePath, guiSelectedMaterial, std::filesystem::path(objFilePath).filename());
    //     //         editingScene->meshes.push_back(m);
    //     //     }
    //     //     ImGui::TreePop();
    //     // }
    // }
    // ImGui::End();

    if(ImGui::Begin("Scenes")) {
        for (size_t i = 0; i < scenes.size(); i++)
        {
            ImGui::PushID(i);
            shared_ptr<Scene> &s = scenes[i];
            if (ImGui::RadioButton(s->name.c_str(), s == scene))
                scene = s;

            ImGui::PopID();
        }
        // if(ImGui::Button("Save"))
        //     serializeEverything("assets/save");
    }
    ImGui::End();

    window.clear();
    ImGui::SFML::Render(window);
    window.display();
}
