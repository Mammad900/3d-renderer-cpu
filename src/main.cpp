#include "data.h"
#include "gui.h"
#include "loadObj.h"
#include "render.h"
#include "generateMesh.h"
#include "sceneFile.h"
#include "earthMaterial.h"
#include <SFML/Graphics.hpp>
#include <filesystem>
int main(int argc, char** argv) {
    materials.push_back(new EarthMaterial());
    parseSceneFile(argc > 1 ? argv[1] : "assets/scene.txt");

    // Scene window
    auto window = sf::RenderWindow(
        sf::VideoMode(frameSize), "3D renderer",
        sf::Style::Titlebar | sf::Style::Close
    );
    window.setFramerateLimit(144);

    // Tools window
    sf::RenderWindow window2(sf::VideoMode({1200, 600}), "Tools");
    window2.setFramerateLimit(60);
    if (!ImGui::SFML::Init(window2))
        return -1;
    sf::Clock deltaClock;

    while (window.isOpen() && window2.isOpen()) {
        guiUpdate(window2, deltaClock); // gui.h

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        lights[0].rotation.x += 0.1;
        objects[0].rotation.y += 0.01;
        render();
        sf::Image img(frameSize);

        for (unsigned int y = 0; y < frameSize.y; y++)
            for (unsigned int x = 0; x < frameSize.x; x++)
                if (renderMode == 0) {// Frame buffer
                    Color pixel = framebuffer[y * frameSize.x + x];
                    img.setPixel({x, y}, pixel.reinhardtTonemap(whitePoint==0?maximumColor:whitePoint, gammaCorrection));
                }
                else if (renderMode == 1) { // Z buffer
                    // Z buffer range is really display-to-end-user unfriendly
                    float z = zBuffer[y * frameSize.x + x] * 20.0f;
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
