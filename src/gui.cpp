#include "gui.h"
#include "camera.h"
#include "data.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "material.h"
#include "phongMaterial.h"
#include "lua/lua.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <iostream>
#include <memory>
#include <string>

char objFilePath[500];
shared_ptr<Material> guiSelectedMaterial;
vector<std::weak_ptr<Material>> materials;
vector<std::weak_ptr<Volume>> volumes;
vector<std::weak_ptr<Mesh>> meshes;
GuiMaterialAssignMode guiMaterialAssignMode;
std::string luaReplInput;

void Timing(Metric<float> &m, const char *name) {
    ImGui::Text("%s: Last %04.1f / Mean %04.1f / Max %04.1f", name, m.last, m.average(), m.maximum);
}

void guiUpdate(shared_ptr<Window> window) {
    shared_ptr<Camera> camera = window->camera;
    shared_ptr<Scene> editingScene = window->scene;

    // ImGui::ShowDemoWindow();

    if(ImGui::Begin("Lua REPL")) {
        ImGui::InputTextMultiline("##", &luaReplInput);
        if(ImGui::Button("Run"))
            luaRun(luaReplInput);
    }
    ImGui::End();

    ImGui::Begin("Options");
    ImGui::InputFloat("Near", &camera->nearClip);
    ImGui::InputFloat("Far", &camera->farClip);
    ImGui::SliderFloat("FOV", &camera->fov, 10, 150);
    ImGui::Checkbox("Orthographic", &camera->orthographic);
    ImGui::RadioButton("Frame buffer", &editingScene->renderMode, 0);
    ImGui::RadioButton("Z buffer", &editingScene->renderMode, 1);
    ImGui::Checkbox("Back-face culling", &editingScene->backFaceCulling);
    ImGui::Checkbox("Reverse all faces", &editingScene->reverseAllFaces);
    ImGui::Checkbox("Full-bright mode", &editingScene->fullBright);
    ImGui::Checkbox("Show wireframe mesh", &editingScene->wireFrame);
    ImGui::Checkbox("Bilinear shadow filtering", &editingScene->bilinearShadowFiltering);
    ImGui::Text("Texture filtering:");
    ImGui::RadioButton("Nearest Neighbor", &editingScene->textureFilteringMode, TextureFilteringMode::NearestNeighbor);
    ImGui::RadioButton("Bilinear", &editingScene->textureFilteringMode, TextureFilteringMode::Bilinear);
    ImGui::RadioButton("Trilinear", &editingScene->textureFilteringMode, TextureFilteringMode::Trilinear);
    ImGui::SliderFloat("White point", (float *)&camera->whitePoint, 0, 5);
    ImGui::End();

    ImGui::Begin("Performance");
    ImGui::Checkbox("Render", &timing.render);
    if(ImGui::Button("Render one frame now and save")) {
        currentWindow = window;
        camera->render();
        sf::Image res = camera->getRenderedFrame(0);
        if(!res.saveToFile("render.png"))
            std::cerr << "Failed to save to render.png" << std::endl;
    }
    ImGui::Text("FPS: %.1f", 1000.f / timing.overallTime.average());
    Timing(timing.overallTime, "Total");
    Timing(timing.windowTime, "Window");
    Timing(timing.updateTime, "Update");
    Timing(timing.skyBoxTime, "SkyBox");
    Timing(timing.renderPrepareTime, "Render prepare");
    Timing(timing.geometryTime, "Geometry");
    Timing(timing.lightingTime, "Lighting");
    Timing(timing.forwardTime, "Forward pass");
    Timing(timing.postProcessTime, "Post processing");
    ImGui::Checkbox("Sync frame size to window size", &window->syncFrameSize);
    if(ImGui::DragScalarN("Frame size", ImGuiDataType_U32, &window->frame->size, 2)) {
        if(window->syncFrameSize)
            window->changeSize(window->frame->size);
        else
            window->changeFrameSize(window->frame->size);
    }
    if(ImGui::Checkbox("Use Deferred rendering", &window->frame->deferred))
        window->frame->changeSize(window->frame->size, window->frame->deferred);
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
                    if(ImGui::ColorEdit4("Transmission", &volume->transmission.r, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR))
                        volume->updateIntensity();
                    ImGui::Checkbox("God-rays", &volume->godRays);
                    ImGui::SliderFloat("Sample size", &volume->godRaysSampleSize, 0.01, 1, "%.3f", ImGuiSliderFlags_Logarithmic);
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

    if(ImGui::Begin("Meshes")) {
        static bool findFaceMode = false;
        ImGui::Checkbox("Find Face Mode", &findFaceMode);

        bool needsCleanup = false;
        for (size_t i = 0; i < meshes.size(); i++) {
            ImGui::PushID(i);
            if(auto mesh = meshes[i].lock()) {
                if(ImGui::TreeNode(mesh->label.c_str())) {
                    ImGui::Text("%lu vertices, %lu faces", mesh->vertices.size(), mesh->faces.size());
                    ImGui::Checkbox("Flat shading", &mesh->flatShading);
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
                    if(ImGui::TreeNode("Faces")) {
                        static Face *highlightedFace = nullptr;
                        static shared_ptr<Material> highlightedMaterial = nullptr;
                        static shared_ptr<Material> highlightMat = std::make_shared<PhongMaterial>(PhongMaterialProps{
                            .emissive = std::make_shared<SolidTexture<Color>>(Color{1,1,0,1})
                        }, (std::string)"Highlight", MaterialFlags{.doubleSided = true});

                        for (uint16_t j = 0; j < mesh->faces.size(); j++) {
                            ImGui::PushID(j);
                            Face &f = mesh->faces[j];
                            uint16_t step = 1;
                            std::string label = std::to_string(j);
                            if(findFaceMode) {
                                ImGui::Text("%s", (label+"               ").c_str());
                                if(ImGui::IsItemHovered()) {
                                    if(highlightedFace) {
                                        highlightedFace->material = highlightedMaterial;
                                    }
                                    highlightedFace = &f;
                                    highlightedMaterial = f.material;
                                    f.material = highlightMat;
                                }
                            } else {
                                ImGui::InputScalarN(label.c_str(), ImGuiDataType_U16, &f.v1, 3, &step);
                                if(ImGui::RadioButton("Highlight", &f == highlightedFace)) {
                                    if(highlightedFace) {
                                        highlightedFace->material = highlightedMaterial;
                                    }
                                    if(&f == highlightedFace)
                                        highlightedFace = nullptr;
                                    else {
                                        highlightedFace = &f;
                                        highlightedMaterial = f.material;
                                        f.material = highlightMat;
                                    }
                                }
                            }
                            ImGui::PopID();
                        }
                        ImGui::TreePop();
                    }
                    ImGui::TreePop();
                }
            } else needsCleanup = true;
            ImGui::PopID();
        }
        if (needsCleanup) {
            meshes.erase(
                std::remove_if(meshes.begin(), meshes.end(),
                            [](auto &w) { return w.expired(); }),
                meshes.end());
        }
    }
    ImGui::End();

    if(ImGui::Begin("Scenes")) {
        for (size_t i = 0; i < scenes.size(); i++) {
            std::weak_ptr<Scene> &s = scenes[i];
            if(shared_ptr<Scene> scene = s.lock()) {
                ImGui::PushID(i);
                ImGui::Text("%s", scene->name.c_str());
                ImGui::PopID();
            }
        }
    }
    ImGui::End();
}
