#include "data.h"

Scene *scene = nullptr;
sf::RenderWindow *renderWindow;
Vector2u frameSize = {500, 500};
Vector2u frameSizeTemp = frameSize;
Color *framebuffer;
float *zBuffer;
std::vector<Scene *> scenes;

void changeFrameSize(sf::Vector2u newSize) {
    if(framebuffer != nullptr)
        delete[] framebuffer;
    if(zBuffer != nullptr)
        delete[] zBuffer;
    framebuffer = new Color[newSize.x * newSize.y];
    zBuffer = new float[newSize.x * newSize.y];
    frameSize = frameSizeTemp = newSize;
    renderWindow->setSize(newSize);
    sf::FloatRect visibleArea({0.f, 0.f}, sf::Vector2f(newSize));
    renderWindow->setView(sf::View(visibleArea));
}