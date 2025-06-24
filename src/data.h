#ifndef __DATA_H__
#define __DATA_H__

#include "object.h"
#include "light.h"
#include "camera.h"
#include <SFML/Graphics.hpp>

using sf::Vector3f, sf::Vector2u;

extern sf::RenderWindow *renderWindow;
struct RenderTarget {
    Vector2u size;
    Color *framebuffer = nullptr;
    float *zBuffer = nullptr;
    Fragment *gBuffer = nullptr;
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
    Metric<float> windowTime, updateTime, skyBoxTime, renderPrepareTime, geometryTime, lightingTime, forwardTime, postProcessTIme;
};

extern FrameTimings timing;

class Camera;

struct Scene {
    std::string name;
    
    std::vector<Light *> lights;
    Color ambientLight = {1, 1, 1, 0.1};

    std::vector<Material*> materials;
    std::vector<Mesh*> meshes;

    std::vector<Object*> objects;

    Camera *camera;
    float maximumColor;

    int renderMode = 0;
    bool backFaceCulling = true;
    bool reverseAllFaces = false;
    bool fullBright = false;
    bool wireFrame = false;
    bool orbit = false;
    bool godRays = false;
    float godRaysSampleSize = 1.0f;
    TextureFilteringMode textureFilteringMode = TextureFilteringMode::NearestNeighbor;
    Color fogColor = {0,0,0,0};
    Texture<Color> *skyBox = new SolidTexture(Color{0,0,0,0});
};

extern Scene *scene;
extern std::vector<Scene *> scenes;
#endif /* __DATA_H__ */
