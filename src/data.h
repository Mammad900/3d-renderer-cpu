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
    Color *framebuffer;
    float *zBuffer;
    Fragment *gBuffer;
    bool deferred;
    void changeSize(sf::Vector2u newSize, bool deferred);

    RenderTarget(Vector2u size, bool deferred = true)
        { changeSize(size, deferred); }
};

void changeWindowSize(Vector2u size);

extern Vector2u frameSizeTemp;
extern RenderTarget *frame;
extern float deltaTime, geometryTime, lightingTime, forwardTime;

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
    TextureFilteringMode textureFilteringMode = TextureFilteringMode::NearestNeighbor;
    Color fogColor = {0,0,0,0};
};

extern Scene *scene;
extern std::vector<Scene *> scenes;
#endif /* __DATA_H__ */
