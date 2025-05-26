#include "data.h"

Scene *scene = nullptr;
sf::RenderWindow *renderWindow;
std::vector<Scene *> scenes;
RenderTarget *frame = new RenderTarget({500, 500});
Vector2u frameSizeTemp = frame->size;

void RenderTarget::changeSize(sf::Vector2u newSize) {
    if(framebuffer != nullptr)
        delete[] framebuffer;
    if(zBuffer != nullptr)
        delete[] zBuffer;
    framebuffer = new Color[newSize.x * newSize.y];
    zBuffer = new float[newSize.x * newSize.y];
    size = frameSizeTemp = newSize;
}

void changeWindowSize(Vector2u newSize) {
    frame->changeSize(newSize);
    renderWindow->setSize(newSize);
    sf::FloatRect visibleArea({0.f, 0.f}, sf::Vector2f(newSize));
    renderWindow->setView(sf::View(visibleArea));
}
