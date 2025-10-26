#include "data.h"

shared_ptr<Scene> scene = nullptr;
sf::RenderWindow *renderWindow;
std::vector<shared_ptr<Scene>> scenes;
RenderTarget *frame = new RenderTarget({500, 500});
Vector2u frameSizeTemp = frame->size;
FrameTimings timing;

void RenderTarget::changeSize(sf::Vector2u newSize, bool deferred) {
    if(framebuffer != nullptr)
        delete[] framebuffer;
    if(zBuffer != nullptr)
        delete[] zBuffer;
    if(gBuffer != nullptr)
        delete[] gBuffer;
    
    if(!shadowMap) // Shadowmaps only have z buffer
        framebuffer = new Color[newSize.x * newSize.y];
    else
        framebuffer = nullptr;

    zBuffer = new float[newSize.x * newSize.y];
    
    if(deferred && !shadowMap)
        gBuffer = new Fragment[newSize.x * newSize.y];
    else
        gBuffer = nullptr;

    this->deferred = deferred;
    size = frameSizeTemp = newSize;
}

void changeWindowSize(Vector2u newSize) {
    frame->changeSize(newSize, frame->deferred);
    // renderWindow->setSize(newSize);
    sf::FloatRect visibleArea({0.f, 0.f}, sf::Vector2f(newSize));
    renderWindow->setView(sf::View(visibleArea));
}
