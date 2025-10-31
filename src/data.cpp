#include "data.h"
#include "color.h"
#include "object.h"
#include <vector>

shared_ptr<Scene> scene = nullptr;
sf::RenderWindow *renderWindow;
std::vector<shared_ptr<Scene>> scenes;
RenderTarget *frame = new RenderTarget({500, 500});
Vector2u frameSizeTemp = frame->size;
FrameTimings timing;

void RenderTarget::changeSize(sf::Vector2u newSize, bool deferred) {
    if(!shadowMap) // Shadowmaps only have z buffer
        framebuffer = vector<Color>(newSize.x * newSize.y);
    else
        framebuffer = vector<Color>(0);

    zBuffer = vector<float>(newSize.x * newSize.y);
    
    if(deferred && !shadowMap)
        gBuffer = vector<Fragment>(newSize.x * newSize.y);
    else
        gBuffer = vector<Fragment>(0);

    this->deferred = deferred;
    size = frameSizeTemp = newSize;
}

void changeWindowSize(Vector2u newSize) {
    frame->changeSize(newSize, frame->deferred);
    // renderWindow->setSize(newSize);
    sf::FloatRect visibleArea({0.f, 0.f}, sf::Vector2f(newSize));
    renderWindow->setView(sf::View(visibleArea));
}
