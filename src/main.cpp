#include "data.h"
#include "gui.h"
#include "loadObj.h"
#include "render.h"
#include "generateMesh.h"
#include <SFML/Graphics.hpp>

int main() {
    // Load teapot
    sf::Image globe("/home/mammad/Documents/3d/assets/globe2.png");
    sf::Image globeS("/home/mammad/Documents/3d/assets/globeSpecular.png");
    sf::Image globeE("/home/mammad/Documents/3d/assets/earth_lights.jpg");
    Material globeMat = {
        .diffuseColor = {1, 1, 1, 1},
        .diffuseTexture = &globe,
        .specularTexture = &globeS,
        .emissiveTexture = &globeE
    };
    materials.push_back(&globeMat);
    meshes.push_back(createSphere(20, 20, &globeMat));
    objects.push_back(Object{
        .mesh = meshes[1],
        .position = {0, 0, 0},
        .rotation = {0, 0, 0},
        .scale = {1, 1, 1}
    });

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

        // objects[0].rotation.y += 0.1;
        render();
        sf::Image img(frameSize);

        for (unsigned int y = 0; y < frameSize.y; y++)
            for (unsigned int x = 0; x < frameSize.x; x++)
                if (renderMode == 0) {// Frame buffer
                    Color pixel = framebuffer[y * frameSize.x + x];
                    img.setPixel({x, y}, pixel.reinhardtTonemap(whitePoint==0?maximumColor:whitePoint).toSFColor());
                }
                else if (renderMode == 1) { // Z buffer
                    // Z buffer range is really display-to-end-user unfriendly
                    float z = 1 / (1 - zBuffer[y * frameSize.x + x]);
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
