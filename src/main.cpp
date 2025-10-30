#include "data.h"
#include "gui.h"
#include "multithreading.h"
#include "lua.h"
#include <SFML/Graphics.hpp>
#include <filesystem>
#include <memory>

int main(int argc, char** argv) {
    lua(argc > 1 ? argv[1] : "assets/scene.lua");

    // Tools window
    sf::RenderWindow window2(sf::VideoMode({1200, 600}), "Tools");
    window2.setFramerateLimit(60);
    if (!ImGui::SFML::Init(window2))
        return -1;
    sf::Clock deltaClock;

    // Scene window
    auto window = sf::RenderWindow(
        sf::VideoMode(frame->size), "3D renderer",
        sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize
    );
    renderWindow = &window;
    window.setFramerateLimit(144);

    while (window.isOpen() && window2.isOpen()) {
        timing.deltaTime = deltaClock.getElapsedTime().asSeconds();
        timing.totalTime += timing.deltaTime;
        timing.clock.restart();

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                changeWindowSize(resized->size);
            }
            if (const auto* pressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                if(pressed->button == sf::Mouse::Button::Left && guiMaterialAssignMode != GuiMaterialAssignMode::None && frame->deferred) {
                    // Find face
                    uint index = pressed->position.x + frame->size.x * pressed->position.y;
                    if(frame->zBuffer[index] != INFINITY)
                        frame->gBuffer[index].face->material = guiSelectedMaterial;
                }
            }
        }
        timing.windowTime.push(timing.clock);

        for (auto &&obj : scene->objects)
            obj->update();

        luaOnFrame();

        guiUpdate(window2, deltaClock, scene); // gui.h

        timing.updateTime.push(timing.clock);

        if(timing.render)
            scene->camera->render();

        timing.clock.restart();

        sf::Texture tex(scene->camera->getRenderedFrame(scene->renderMode));
        sf::Sprite spr(tex);
        window.clear();
        window.draw(spr);
        window.display();

        timing.postProcessTIme.push(timing.clock);
    }
    luaDestroy();
    ImGui::SFML::Shutdown();
    shutdownThreads();
}
