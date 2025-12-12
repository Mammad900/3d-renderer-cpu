#ifndef __DATA_H__
#define __DATA_H__

#include "color.h"
#include "material.h"
#include "object.h"
#include "light.h"
#include "camera.h"
#include <SFML/Graphics.hpp>
#include "environmentMap.h"
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

using sf::Vector2u, std::shared_ptr;

struct FragmentNode {
    Fragment f;
    uint32_t next;
};

struct RenderTarget {
    Vector2u size;
    vector<Color> framebuffer;
    vector<float> zBuffer;
    vector<Fragment> gBuffer;
    vector<FragmentNode> transparencyFragments;
    vector<uint32_t> transparencyHeads;
    bool deferred, shadowMap;
    void changeSize(sf::Vector2u newSize, bool deferred);

    RenderTarget(Vector2u size, bool deferred = true, bool shadowMap = false) : shadowMap(shadowMap)
        { changeSize(size, deferred); }
};

template<typename T>
struct Metric {
    std::vector<T> history = std::vector<T>(100, 0);
    T last, total, maximum;
    int n = 0;
    T average() { return total / 100; }
    void push(T value) {
        total -= history[n];
        total += last = history[n] = value;
        n = (n + 1) % 100;
        maximum = 0;
        for (auto &&v : history)
            if(v > maximum)
                maximum = v;
    }
    void push(sf::Clock &cl) {
        push(cl.restart().asSeconds() * 1000.0f);
    }
};

struct FrameTimings {
    sf::Clock clock;
    float deltaTime;
    float totalTime = 0;
    Metric<float> windowTime, updateTime, skyBoxTime, renderPrepareTime,
        geometryTime, lightingTime, forwardTime, postProcessTime, overallTime;
    bool render = true;
};

extern FrameTimings timing;

class Camera;

struct Scene : public std::enable_shared_from_this<Scene> {
    std::string name;
    bool shouldUpdate = false;
    bool alwaysUpdate = false;
    
    std::vector<Light *> lights;
    Color ambientLight = {1, 1, 1, 0.1};

    std::vector<shared_ptr<Object>> objects;

    int renderMode = 0;
    bool backFaceCulling = true;
    bool reverseAllFaces = false;
    bool fullBright = false;
    bool wireFrame = false;
    bool bilinearShadowFiltering = true;
    float shadowBias = 0.1f;
    TextureFilteringMode textureFilteringMode = TextureFilteringMode::NearestNeighbor;

    shared_ptr<Volume> volume;
    shared_ptr<EnvironmentMap> skyBox = std::make_shared<SolidEnvironmentMap>(Color{0, 0, 0, 0});
};

class Window {
  public:
    sf::RenderWindow window;
    bool quitWhenClosed = false;
    std::shared_ptr<RenderTarget> frame;
    shared_ptr<Camera> camera;
    shared_ptr<Scene> scene;
    shared_ptr<Window> toolWindowFor;
    bool hasGui = false;
    std::string name;
    Vector2u size{500, 500};
    bool syncFrameSize = true;
    std::function<void()> gui;
    std::function<void(std::optional<sf::Event>)> onEvent;
    
    void changeSize(Vector2u newSize);
    void changeFrameSize(Vector2u newSize);

    void init();
};

extern std::vector<std::weak_ptr<Scene>> scenes;
extern std::shared_ptr<Window> currentWindow;
extern bool initComplete;
#endif /* __DATA_H__ */
