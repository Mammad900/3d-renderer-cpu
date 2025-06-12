#include "data.h"

Scene *scene = nullptr;
sf::RenderWindow *renderWindow;
std::vector<Scene *> scenes;
RenderTarget *frame = new RenderTarget({500, 500});
Vector2u frameSizeTemp = frame->size;
float deltaTime;

void RenderTarget::changeSize(sf::Vector2u newSize, bool deferred) {
    if(framebuffer != nullptr)
        delete[] framebuffer;
    if(zBuffer != nullptr)
        delete[] zBuffer;
    if(gBuffer != nullptr)
        delete[] gBuffer;
    framebuffer = new Color[newSize.x * newSize.y];
    zBuffer = new float[newSize.x * newSize.y];
    if(deferred)
        gBuffer = new Fragment[newSize.x * newSize.y];
    else
        gBuffer = nullptr;
    this->deferred = deferred;
    size = frameSizeTemp = newSize;
}

void changeWindowSize(Vector2u newSize) {
    frame->changeSize(newSize, frame->deferred);
    renderWindow->setSize(newSize);
    sf::FloatRect visibleArea({0.f, 0.f}, sf::Vector2f(newSize));
    renderWindow->setView(sf::View(visibleArea));
}
