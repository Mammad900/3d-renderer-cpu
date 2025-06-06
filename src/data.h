#ifndef __DATA_H__
#define __DATA_H__

#include "object.h"
// #include "light.h"
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

struct Scene {
    std::string name;
    
    std::vector<Light> lights;
    Color ambientLight = {1, 1, 1, 0.1};

    std::vector<Material*> materials;
    std::vector<Mesh*> meshes;

    std::vector<Object*> objects;

    Vector3f cam = {0, 0, 0};
    Vector3f camRotation = {0, 0, 0};
    Vector3f camDirection;
    float nearClip = 0.1, farClip = 100;
    float fov = 90;
    TransformMatrix projectionMatrix;

    float maximumColor;

    int renderMode = 0;
    bool backFaceCulling = true;
    bool reverseAllFaces = false;
    bool fullBright = false;
    bool wireFrame = false;
    bool orbit = false;
    TextureFilteringMode textureFilteringMode = TextureFilteringMode::NearestNeighbor;
    float whitePoint = 1;
    Color fogColor = {0,0,0,0};
};

extern Scene *scene;
extern std::vector<Scene *> scenes;
#endif /* __DATA_H__ */
