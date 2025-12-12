#include "data.h"
#include "gui.h"
#include "multithreading.h"
#include "main.h"
#include "lua.h"
#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include <memory>
#include <string>

std::vector<std::shared_ptr<Window>> windows;

int main(int argc, char** argv) {
    lua(argc > 1 ? argv[1] : "assets/scene.lua");

    sf::Clock deltaClock;

    for (auto &&window : windows) {
        window->init();
    }

    while (windows.size() > 0) {
        timing.deltaTime = deltaClock.getElapsedTime().asSeconds();
        timing.totalTime += timing.deltaTime;
        timing.clock.restart();

        windows.erase(
            std::remove_if(windows.begin(), windows.end(),
                        [](auto &w) { return !w->window.isOpen(); }),
            windows.end()
        );
        scenes.erase(
            std::remove_if(scenes.begin(), scenes.end(),
                        [](auto &s) { return s.expired(); }),
            scenes.end()
        );

        for (auto && window : windows) {
            if(window->scene)
                window->scene->shouldUpdate = true;
            while (const std::optional event = window->window.pollEvent()) {
                if (event->is<sf::Event::Closed>()) {
                    window->window.close();
                    if(window->quitWhenClosed) {
                        windows.clear();
                    }
                }
                if(window->toolWindowFor) {
                    ImGui::SFML::ProcessEvent(window->window, *event);
                }
                if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                    window->changeSize(resized->size);
                }
                if (const auto* pressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if(auto frame = window->frame) {
                        if(pressed->button == sf::Mouse::Button::Left && guiMaterialAssignMode != GuiMaterialAssignMode::None && frame->deferred) {
                            // Find face
                            uint index = pressed->position.x + frame->size.x * pressed->position.y;
                            if(frame->zBuffer[index] != INFINITY)
                                frame->gBuffer[index].face->material = guiSelectedMaterial;
                        }
                    }
                }
            }
        }

        for (auto &&s : scenes) {
            auto scene = s.lock();
            if(scene->alwaysUpdate || scene->shouldUpdate) {
                for (auto &&obj : scene->objects)
                    obj->update();
                scene->shouldUpdate = false;
            }
        }
        luaOnFrame();
        timing.updateTime.push(timing.clock);
        deltaClock.restart();

        for (auto &&window : windows) {
            currentWindow = window;
            window->window.clear();
            if(window->frame) {
                if(timing.render)
                    window->camera->render();

                timing.clock.restart();

                sf::Texture tex(window->camera->getRenderedFrame(window->scene->renderMode));
                sf::Sprite spr(tex);
                window->window.draw(spr);
            }
            if(window->hasGui) {
                ImGui::SFML::Update(window->window, deltaClock.getElapsedTime());
                if(window->toolWindowFor)
                    guiUpdate(window->toolWindowFor);
                ImGui::SFML::Render(window->window);
            }
            window->window.display();
            timing.postProcessTime.push(timing.clock);
        }

    }
    luaDestroy();
    ImGui::SFML::Shutdown();
    shutdownThreads();
}
