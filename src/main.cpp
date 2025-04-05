#include <SFML/Graphics.hpp>
#include "render.h"
#include "data.h"
#include "gui.h"
#include "loadObj.h"

int main()
{
    meshes.push_back(loadOBJ("/home/mammad/Documents/3d/assets/teapot.obj"));
    objects.push_back(Object{.mesh = meshes[1], .position = {0, 0, 0}, .rotation = {0, 0, 0}, .scale = {1, 1, 1}});
    auto window = sf::RenderWindow(sf::VideoMode(frameSize), "3D renderer", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(144);

    sf::RenderWindow window2(sf::VideoMode({800, 600}), "Tools");
    window2.setFramerateLimit(60);
    if (!ImGui::SFML::Init(window2))
        return -1;
    sf::Clock deltaClock;

    while (window.isOpen() && window2.isOpen())
    {
        guiUpdate(window2, deltaClock);
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)){
                cam.y += 0.1;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Add)){
                fov -= 5;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::M)){
                renderMode = (renderMode+1) % 2;
            }
        }

        objects[0].rotation.y += 0.1;
        sf::Image img({500, 500});
        render();
        for (unsigned int y = 0; y < 500; y++)
            for (unsigned int x = 0; x < 500; x++){
                if(renderMode==0)
                    img.setPixel({x, y}, framebuffer[y * 500 + x].toSFColor());
                else if(renderMode==1){
                    volatile float z = 1/(1-zBuffer[y * 500 + x]);
                    // std::cout << z << std::endl;
                    img.setPixel({x, y}, sf::Color(z, z, z));
                }
            }
        sf::Texture tex(img);
        sf::Sprite spr(tex);
        window.clear();
        window.draw(spr);
        window.display();
    }
    ImGui::SFML::Shutdown();
}
