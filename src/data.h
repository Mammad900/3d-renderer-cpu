#ifndef __DATA_H__
#define __DATA_H__

#include "color.h"
#include "material.h"
#include "object.h"
#include "light.h"
#include "camera.h"
#include <SFML/Graphics.hpp>
#include "environmentMap.h"
#include <memory>
#include <vector>

using sf::Vector2u, std::shared_ptr;

extern sf::RenderWindow *renderWindow;
struct RenderTarget {
    Vector2u size;
    vector<Color> framebuffer;
    vector<float> zBuffer;
    vector<Fragment> gBuffer;
    bool deferred, shadowMap;
    void changeSize(sf::Vector2u newSize, bool deferred);

    RenderTarget(Vector2u size, bool deferred = true, bool shadowMap = false) : shadowMap(shadowMap)
        { changeSize(size, deferred); }
};

void changeWindowSize(Vector2u size);

extern Vector2u frameSizeTemp;
extern RenderTarget *frame;

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
        geometryTime, lightingTime, forwardTime, postProcessTIme;
    bool render = true;
};

extern FrameTimings timing;

class Camera;

struct Scene : public std::enable_shared_from_this<Scene> {
    std::string name;
    
    std::vector<Light *> lights;
    Color ambientLight = {1, 1, 1, 0.1};

    std::vector<shared_ptr<Object>> objects;

    shared_ptr<Camera> camera;
    float maximumColor;

    int renderMode = 0;
    bool backFaceCulling = true;
    bool reverseAllFaces = false;
    bool fullBright = false;
    bool wireFrame = false;
    bool godRays = false;
    float godRaysSampleSize = 1.0f;
    bool bilinearShadowFiltering = true;
    float shadowBias = 0.1f;
    TextureFilteringMode textureFilteringMode = TextureFilteringMode::NearestNeighbor;

    shared_ptr<Volume> volume;
    shared_ptr<EnvironmentMap> skyBox = std::make_shared<SolidEnvironmentMap>(Color{0, 0, 0, 0});

    void setActiveCamera(shared_ptr<Camera> camera) {
        if(this->camera)
            this->camera->tFrame = nullptr;
        this->camera = camera;
        camera->tFrame = frame;
    }
};

extern shared_ptr<Scene> scene;
extern std::vector<shared_ptr<Scene>> scenes;
#endif /* __DATA_H__ */
