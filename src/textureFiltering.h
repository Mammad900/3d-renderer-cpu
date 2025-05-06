#ifndef __TEXTUREFILTERING_H__
#define __TEXTUREFILTERING_H__

#include <math.h>
#include "color.h"
#include "object.h"
#include <SFML/Graphics.hpp>

using sf::Vector2f, std::floor, std::ceil, std::max, std::clamp;

template <typename T>
inline T lerp2d(T a, T b, T c, T d, float t1, float t2) {
    return (b * t1 + a * (1 - t1)) * (1 - t2) +
           (d * t1 + c * (1 - t1)) * t2;
}

template <typename T>
Vector2f textureCoordinates(Texture<T> &texture, Vector2f &uv, Vector2u mipLevel) {
    float pos3x = clamp(uv.x, 0.0f, 1.0f) * (texture.size.x / powf(2, mipLevel.x) -1) + (int)(texture.atlasSize.x - powf(2, texture.mipCount.x - mipLevel.x+1)+1);
    float pos3y = clamp(uv.y, 0.0f, 1.0f) * (texture.size.y / powf(2, mipLevel.y) -1) + (int)(texture.atlasSize.y - powf(2, texture.mipCount.y - mipLevel.y+1)+1);
    return {pos3x, pos3y};
}

template <typename T>
const T textureBilinearFilter(Texture<T> &texture, sf::Vector2f &uv, sf::Vector2u &mipLevel) {
    Vector2f pos = textureCoordinates(texture, uv, mipLevel);
    float decimalsX = pos.x - floor(pos.x);
    float decimalsY = pos.y - floor(pos.y);
    T s1 = texture.pixels[(int)floor(pos.x) + texture.atlasSize.x * (int)floor(pos.y)];
    T s2 = texture.pixels[(int)floor(pos.x) + texture.atlasSize.x * (int)ceil(pos.y)];
    T s3 = texture.pixels[(int)ceil(pos.x) + texture.atlasSize.x * (int)floor(pos.y)];
    T s4 = texture.pixels[(int)ceil(pos.x) + texture.atlasSize.x * (int)ceil(pos.y)];
    return lerp2d(s1, s2, s3, s4, decimalsY, decimalsX);
}

template <typename T>
T textureSample(
    Texture<T> &texture, Vector2f uv, Vector2f dUVdx, Vector2f dUVdy
) {
    // Check mip level
    float rho = max(dUVdx.length(), dUVdy.length());
    Vector2f mipLevelF{
        log2(rho * texture.size.x),
        log2(rho * texture.size.y),
    };
    Vector2u mipLevel{
        clamp((uint)floor(mipLevelF.x), 0u, texture.mipCount.x),
        clamp((uint)floor(mipLevelF.y), 0u, texture.mipCount.y),
    };

    // Bilinear filtering
    if(textureFilteringMode == 1) {
        return textureBilinearFilter(texture, uv, mipLevel);
    }
    // Trilinear (blend mipmaps)
    if(textureFilteringMode == 2) {
        Vector2u mipLevel2{
            clamp((uint)ceil(mipLevelF.x), 0u, texture.mipCount.x),
            clamp((uint)ceil(mipLevelF.y), 0u, texture.mipCount.y),
        };
        float t = mipLevelF.x - floor(mipLevelF.x);
        return textureBilinearFilter(texture, uv, mipLevel) * (1-t) +
               textureBilinearFilter(texture, uv, mipLevel2) * t;
    }
    // Nearest Neighbor
    else {
        Vector2f pos = textureCoordinates(texture, uv, mipLevel);
        return texture.pixels[(int)round(pos.x) + texture.atlasSize.x * (int)round(pos.y)];
    }
}


template <typename T>
void generateMipmaps(T *pixels, Vector2u size, Vector2u atlasSize) {
    if((size.x & (size.x - 1)) || (size.y & (size.y - 1))) {
        std::cerr << "Texture size is not power of 2" << std::endl;
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
    Vector2u mipCount = {(uint)log2(size.x), (uint)log2(size.y)};
    Color *pixels = new Color[atlasSize.x*atlasSize.y];
    for (uint y = 0; y < size.y; y++)
        for (uint x = 0; x < size.x; x++)
            pixels[y * atlasSize.x + x] = Color::fromSFColor(img.getPixel({x, y}));
    generateMipmaps(pixels, size, atlasSize);
    return {pixels, size, atlasSize, mipCount};
}

Texture<Vector3f> loadVectorTexture(sf::Image &img) {
    Vector2u size = img.getSize();
    Vector2u atlasSize = {size.x * 2 - 1, size.y * 2 - 1};
    Vector2u mipCount = {(uint)log2(size.x), (uint)log2(size.y)};
    Vector3f *pixels = new Vector3f[atlasSize.x * atlasSize.y];
    for (uint y = 0; y < size.y; y++)
        for (uint x = 0; x < size.x; x++)
            pixels[y * atlasSize.x + x] = Color::fromSFColor(img.getPixel({x, y}));
    generateMipmaps(pixels, size, atlasSize);
    return {pixels, size, atlasSize, mipCount};
}

Texture<float> loadFloatTexture(sf::Image &img) {
    Vector2u size = img.getSize();
    Vector2u atlasSize = {size.x * 2 - 1, size.y * 2 - 1};
    Vector2u mipCount = {(uint)log2(size.x), (uint)log2(size.y)};
    float *pixels = new float[atlasSize.x*atlasSize.y];
    for (uint y = 0; y < size.y; y++)
        for (uint x = 0; x < size.x; x++)
            pixels[y * atlasSize.x + x] = img.getPixel({x, y}).a / 255.0f;
    generateMipmaps(pixels, size, atlasSize);
    return {pixels, size, atlasSize, mipCount};
}

#endif /* __TEXTUREFILTERING_H__ */
