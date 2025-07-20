#include "data.h"
#include "gui.h"
#include "generateMesh.h"
#include "sceneFile.h"
#include <SFML/Graphics.hpp>
#include <filesystem>
int main(int argc, char** argv) {
    parseSceneFile(argc > 1 ? argv[1] : "assets/scene.txt", scene);

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
        timing.clock.restart();
        guiUpdate(window2, deltaClock, scene); // gui.h

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

        if (scene->orbit) {
            scene->lights[0]->obj->rotation.y += 0.1;
            scene->objects[0]->rotation.y += 0.01;
        }
        if(window.hasFocus() && scene->keyboardControl) {
            Vec3 speed = timing.deltaTime * scene->keyboardControl->speed;
            Object *obj = scene->keyboardControl->obj;
            using sf::Keyboard::Key, sf::Keyboard::isKeyPressed;
            if(isKeyPressed(Key::Right))
                obj->rotation.y += speed.x;
            if(isKeyPressed(Key::Left))
                obj->rotation.y -= speed.x;
            if(isKeyPressed(Key::Up))
                obj->rotation.x += speed.y;
            if(isKeyPressed(Key::Down))
                obj->rotation.x -= speed.y;
            if(isKeyPressed(Key::Add))
                obj->scale *= 1 + speed.z;
            if(isKeyPressed(Key::Subtract))
                obj->scale *= 1 - speed.z;
            obj->rotation.x = std::clamp(obj->rotation.x, -M_PI_2f, M_PI_2f);
        }

        for (auto &&obj : scene->objects)
            obj->update();

        timing.updateTime.push(timing.clock);

        scene->camera->render();

        timing.clock.restart();

        sf::Image img(frame->size);
        for (unsigned int y = 0; y < frame->size.y; y++)
            for (unsigned int x = 0; x < frame->size.x; x++)
                if (scene->renderMode == 0) {// Frame buffer
                    Color pixel = frame->framebuffer[y * frame->size.x + x];
                    img.setPixel({x, y}, pixel.reinhardtTonemap(scene->camera->whitePoint==0?scene->maximumColor:scene->camera->whitePoint));
                }
                else if (scene->renderMode == 1) { // Z buffer
                    // Z buffer range is really display-to-end-user unfriendly
                    float z = frame->zBuffer[y * frame->size.x + x] * 20.0f;
                    img.setPixel({x, y}, sf::Color(z, z, z));
                }

        sf::Texture tex(img);
        sf::Sprite spr(tex);
        window.clear();
        window.draw(spr);
        window.display();

        timing.postProcessTIme.push(timing.clock);
    }
    ImGui::SFML::Shutdown();
    shutdownThreads();
}
