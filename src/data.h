#ifndef __DATA_H__
#define __DATA_H__

#include "object.h"
#include <SFML/Graphics.hpp>

using sf::Vector3f, sf::Vector2u;

extern sf::RenderWindow *renderWindow;
extern Vector2u frameSize;
extern Vector2u frameSizeTemp;
extern Color *framebuffer;
extern float *zBuffer;
void changeFrameSize(sf::Vector2u newSize);

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
    float projectionMatrix[16];

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

extern Scene *scene;
#endif /* __DATA_H__ */
