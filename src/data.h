#ifndef __DATA_H__
#define __DATA_H__

#include "object.h"
#include <SFML/Graphics.hpp>

using sf::Vector3f, sf::Vector2u;

sf::RenderWindow *renderWindow;
Vector2u frameSize = {500, 500};
Vector2u frameSizeTemp = frameSize;
Color *framebuffer;
float *zBuffer;
void changeFrameSize(sf::Vector2u newSize) {
    if(framebuffer != nullptr)
        delete[] framebuffer;
    if(zBuffer != nullptr)
        delete[] zBuffer;
    framebuffer = new Color[newSize.x * newSize.y];
    zBuffer = new float[newSize.x * newSize.y];
    frameSize = frameSizeTemp = newSize;
    renderWindow->setSize(newSize);
    sf::FloatRect visibleArea({0.f, 0.f}, sf::Vector2f(newSize));
    renderWindow->setView(sf::View(visibleArea));
}

struct Scene {
    std::vector<Light> lights;
    Color ambientLight = {1, 1, 1, 0.1};

    std::vector<Material*> materials;
    std::vector<Mesh*> meshes;

    std::vector<Object> objects;

    Vector3f cam = {0, 0, 0};
    Vector3f camRotation = {0, 0, 0};
    Vector3f camDirection;
    float nearClip = 0.1, farClip = 100;
    float fov = 90;

    float maximumColor;

    int renderMode = 0;
    bool backFaceCulling = true;
    bool reverseAllFaces = false;
    bool fullBright = false;
    bool wireFrame = false;
    bool orbit = false;
    int textureFilteringMode = 0;
    float whitePoint = 1;
    Color fogColor = {0,0,0,0};
};

Scene *scene = new Scene();
#endif /* __DATA_H__ */
