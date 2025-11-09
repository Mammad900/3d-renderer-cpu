#include "data.h"
#include "color.h"
#include "object.h"
#include <cstddef>
#include <cstdint>
#include <vector>

shared_ptr<Scene> scene = nullptr;
sf::RenderWindow *renderWindow;
std::vector<shared_ptr<Scene>> scenes;
RenderTarget *frame = new RenderTarget({500, 500});
Vector2u frameSizeTemp = frame->size;
FrameTimings timing;

void RenderTarget::changeSize(sf::Vector2u newSize, bool deferred) {
    size_t n = newSize.x * newSize.y;

    framebuffer = vector<Color>(shadowMap ? 0 : n); // Shadowmaps only have z buffer
    zBuffer = vector<float>(n);
    bool useGBuffer = deferred && !shadowMap;
    gBuffer = vector<Fragment>(useGBuffer ? n : 0);
    transparencyHeads = vector<uint32_t>(useGBuffer ? n : 0);

    this->deferred = deferred;
    size = frameSizeTemp = newSize;
}

void changeWindowSize(Vector2u newSize) {
    frame->changeSize(newSize, frame->deferred);
    // renderWindow->setSize(newSize);
    sf::FloatRect visibleArea({0.f, 0.f}, sf::Vector2f(newSize));
    renderWindow->setView(sf::View(visibleArea));
}
