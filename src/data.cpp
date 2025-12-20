#include "data.h"
#include "color.h"
#include "object.h"
#include "gui.h"
#include <vector>

std::vector<std::weak_ptr<Scene>> scenes;
FrameTimings timing;
std::shared_ptr<Window> currentWindow;
bool initComplete = false;

void RenderTarget::changeSize(sf::Vector2u newSize, bool deferred) {
    size_t n = newSize.x * newSize.y;

    framebuffer = vector<Color>(shadowMap ? 0 : n); // Shadowmaps only have z buffer
    zBuffer = vector<float>(n);
    bool useGBuffer = deferred && !shadowMap;
    gBuffer = vector<Fragment>(useGBuffer ? n : 0);
    transparencyHeads = vector<uint32_t>(useGBuffer ? n : 0);

    this->deferred = deferred;
    size = newSize;
}

void Window::init()  {
    window = sf::RenderWindow(
        sf::VideoMode(size), name,
        sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize
    );
    window.setVerticalSyncEnabled(true);
    if(hasGui && !ImGui::SFML::Init(window))
        throw std::runtime_error("Could not init ImGui for window: " + name);
}

void Window::changeSize(Vector2u newSize) {
    size = newSize;
    window.setSize(newSize);
    if(syncFrameSize && frame) {
        changeFrameSize(newSize);
    }
}

void Window::changeFrameSize(Vector2u newSize) {
    frame->changeSize(newSize, frame->deferred);
    sf::FloatRect visibleArea({0.f, 0.f}, sf::Vector2f(newSize));
    window.setView(sf::View(visibleArea));
}
