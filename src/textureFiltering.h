#ifndef __TEXTUREFILTERING_H__
#define __TEXTUREFILTERING_H__

#include <math.h>
#include "color.h"
#include "object.h"
#include <SFML/Graphics.hpp>

using sf::Vector2f;

template <typename T>
T textureFilter(Texture<T> &texture, Vector2f pos) {
    Vector2u pPos = {
        std::clamp((uint)round(pos.x * texture.size.x), 0u, texture.size.x - 1),
        std::clamp((uint)round(pos.y * texture.size.y), 0u, texture.size.y - 1)
    };
    return texture.pixels[pPos.x + texture.atlasSize.x * pPos.y];
}

template <typename T>
void generateMipmaps(T *pixels, Vector2u size, Vector2u atlasSize) {
    if((size.x & (size.x - 1)) || (size.y & (size.y - 1))) {
        std::cout << "Texture size is not power of 2" << std::endl;
        return;
    }

    // We assume that the topleft mipmap (full size) is already written by caller

    #define TEXEL(xx, yy) (pixels[(xx) + atlasSize.x * (yy)]) 

    // w is width of mipmap being made. xx is position of mipmap being read. My most cursed for loop yet.
    for (uint w = size.x/2, xx = 0; w>0; xx+= w*2, w/= 2)
        for (uint y = 0; y < size.y; y++)
            for (uint x = 0; x < w; x++)
                TEXEL(xx+x+w*2, y) = (TEXEL(xx+x*2, y)+TEXEL(xx+x*2+1, y)) / 2.0f; // Do not attempt to understand this

    // h is height of row being made. yy is position of row being read.
    for (uint h = size.y/2, yy = 0; h>0; yy+= h*2, h/= 2)
        for (uint y = 0; y < h; y++)
            for (uint x = 0; x < atlasSize.x; x++)
                TEXEL(x, yy+h*2+y) = (TEXEL(x, yy+y*2)+TEXEL(x, yy+y*2+1)) / 2.0f; // Or this

    #undef TEXEL
}

Texture<Color> loadColorTexture(sf::Image &img) {
    Vector2u size = img.getSize();
    Vector2u atlasSize = {size.x * 2 - 1, size.y * 2 - 1};
    Color *pixels = new Color[atlasSize.x*atlasSize.y];
    for (uint y = 0; y < size.y; y++)
        for (uint x = 0; x < size.x; x++)
            pixels[y * atlasSize.x + x] = Color::fromSFColor(img.getPixel({x, y}));
    // generateMipmaps(pixels, size, atlasSize);
    return {pixels, size, atlasSize};
}

Texture<Vector3f> loadVectorTexture(sf::Image &img) {
    Vector2u size = img.getSize();
    Vector2u atlasSize = {size.x * 2 - 1, size.y * 2 - 1};
    Vector3f *pixels = new Vector3f[atlasSize.x*atlasSize.y];
    for (uint y = 0; y < size.y; y++)
        for (uint x = 0; x < size.x; x++)
            pixels[y * atlasSize.x + x] = Color::fromSFColor(img.getPixel({x, y}));
    // generateMipmaps(pixels, size, atlasSize);
    return {pixels, size, atlasSize};
}

Texture<float> loadFloatTexture(sf::Image &img) {
    Vector2u size = img.getSize();
    Vector2u atlasSize = {size.x * 2 - 1, size.y * 2 - 1};
    float *pixels = new float[atlasSize.x*atlasSize.y];
    for (uint y = 0; y < size.y; y++)
        for (uint x = 0; x < size.x; x++)
            pixels[y * atlasSize.x + x] = img.getPixel({x, y}).a / 255.0f;
    // generateMipmaps(pixels, size, atlasSize);
    return {pixels, size, atlasSize};
}

#endif /* __TEXTUREFILTERING_H__ */
