#include "data.h"
#include "gui.h"
#include "generateMesh.h"
#include "sceneFile.h"
#include <SFML/Graphics.hpp>
#include <filesystem>
int main(int argc, char** argv) {
    parseSceneFile(argc > 1 ? argv[1] : "assets/scene.txt", scene);

    // Scene window
    auto window = sf::RenderWindow(
        sf::VideoMode(frame->size), "3D renderer",
        sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize
    );
    renderWindow = &window;
    window.setFramerateLimit(144);

    // Tools window
    sf::RenderWindow window2(sf::VideoMode({1200, 600}), "Tools");
    window2.setFramerateLimit(60);
    if (!ImGui::SFML::Init(window2))
        return -1;
    sf::Clock deltaClock;

    while (window.isOpen() && window2.isOpen()) {
        guiUpdate(window2, deltaClock, scene); // gui.h

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            if (const auto* resized = event->getIf<sf::Event::Resized>())
            {
                changeWindowSize(resized->size);
            }
        }

        if(scene->orbit) {
            scene->lights[0]->obj->rotation.y += 0.1;
            scene->objects[0]->rotation.y += 0.01;
        }

        for (auto &&obj : scene->objects)
            obj->update();
        scene->camera->render(frame);

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
    }
    ImGui::SFML::Shutdown();
}
