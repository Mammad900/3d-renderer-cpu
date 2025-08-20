#ifndef __TEXTUREFILTERING_H__
#define __TEXTUREFILTERING_H__

#include <math.h>
#include "color.h"
#include "object.h"
#include "data.h"
#include "texture.h"
#include <SFML/Graphics.hpp>
#include <iostream>

using sf::Vector2f, std::floor, std::ceil, std::max, std::clamp;

template <typename T> 
inline T lerp2d(T a, T b, T c, T d, float t1, float t2) {
    return (b * t1 + a * (1 - t1)) * (1 - t2) +
           (d * t1 + c * (1 - t1)) * t2;
}

template <typename T>
class ImageTexture : public SolidTexture<T> {
public:
    Vector2u size;
    TextureFilteringMode filteringMode;
    ImageTexture(sf::Image &img, T scale, TextureFilteringMode overrideFilteringMode = TextureFilteringMode::None) 
    : SolidTexture<T>(scale), filteringMode(overrideFilteringMode) {
        size = img.getSize();
        atlasSize = {size.x * 2 - 1, size.y * 2 - 1};
        mipCount = {(int)log2(size.x), (int)log2(size.y)};
        pixels = new T[atlasSize.x * atlasSize.y];
        for (uint y = 0; y < size.y; y++)
            for (uint x = 0; x < size.x; x++)
                pixels[y * atlasSize.x + x] = Color::fromSFColor(img.getPixel({x, y}));
        generateMipmaps();
    }

    sf::Image saveToImage() const {
        sf::Image img(size);

        for (uint y = 0; y < size.y; ++y) {
            for (uint x = 0; x < size.x; ++x) {
                T pixel = pixels[y * atlasSize.x + x];
                Color color;

                if constexpr (std::is_same_v<T, Color>) {
                    color = pixel;
                } else if constexpr (std::is_same_v<T, float>) {
                    color = {0, 0, 0, pixel};
                } else if constexpr (std::is_same_v<T, Vec3>) {
                    pixel = (pixel.componentWiseMul({-1,-1,1}) + Vec3{1,1,1}) / 2.0f;
                    color = {pixel.x, pixel.y, pixel.z, 1};
                } else {
                    std::cerr << "Unsupported texture type for saving to image." << std::endl;
                    continue;
                }

                // Using the operator overload doesn't work because it doesn't preserve alpha
                // It doesn't preserve alpha because it would break framebuffer
                uint8_t r = std::clamp((int)(color.r * 255), 0, 255);
                uint8_t g = std::clamp((int)(color.g * 255), 0, 255);
                uint8_t b = std::clamp((int)(color.b * 255), 0, 255);
                uint8_t a = std::clamp((int)(color.a * 255), 0, 255);
                img.setPixel({x, y}, {r, g, b, a});
            }
        }

        return img;
    }

private:
    T* pixels;
    Vector2u atlasSize;
    Vector2i mipCount;


    void generateMipmaps() {
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
    }

    Vector2f getCoordinates(Vector2f &uv, Vector2u mipLevel) {
        float pos3x = clamp(uv.x, 0.0f, 1.0f) * (size.x / powf(2, mipLevel.x) -1) + (int)(atlasSize.x - powf(2, mipCount.x - mipLevel.x+1)+1);
        float pos3y = clamp(uv.y, 0.0f, 1.0f) * (size.y / powf(2, mipLevel.y) -1) + (int)(atlasSize.y - powf(2, mipCount.y - mipLevel.y+1)+1);
        return {pos3x, pos3y};
    }
    
    const T bilinearFilter(sf::Vector2f &uv, sf::Vector2u &mipLevel) {
        Vector2f pos = getCoordinates(uv, mipLevel);
        float decimalsX = pos.x - floor(pos.x);
        float decimalsY = pos.y - floor(pos.y);
        T s1 = pixels[(int)floor(pos.x) + atlasSize.x * (int)floor(pos.y)];
        T s2 = pixels[(int)floor(pos.x) + atlasSize.x * (int)ceil(pos.y)];
        T s3 = pixels[(int)ceil(pos.x) + atlasSize.x * (int)floor(pos.y)];
        T s4 = pixels[(int)ceil(pos.x) + atlasSize.x * (int)ceil(pos.y)];
        return lerp2d(s1, s2, s3, s4, decimalsY, decimalsX);
    }

public:
    T sample(Vector2f uv, Vector2f dUVdx, Vector2f dUVdy) {
        TextureFilteringMode mode =
            filteringMode == TextureFilteringMode::None ? scene->textureFilteringMode : filteringMode;
        // Check mip level
        float rho = max(dUVdx.length(), dUVdy.length());
        Vector2f mipLevelF{
            log2(rho * size.x),
            log2(rho * size.y),
        };
        Vector2u mipLevel{
            (uint)clamp((int)floor(mipLevelF.x), 0, mipCount.x),
            (uint)clamp((int)floor(mipLevelF.y), 0, mipCount.y),
        };

        T res;

        // Bilinear filtering
        if(mode == TextureFilteringMode::Bilinear) {
            res = bilinearFilter(uv, mipLevel);
        }
        // Trilinear (blend mipmaps)
        else if(mode == TextureFilteringMode::Trilinear) {
            Vector2u mipLevel2{
                (uint)clamp((int)ceil(mipLevelF.x), 0, mipCount.x),
                (uint)clamp((int)ceil(mipLevelF.y), 0, mipCount.y),
            };
            float t = mipLevelF.x - floor(mipLevelF.x);
            res = bilinearFilter(uv, mipLevel) * (1-t) +
                  bilinearFilter(uv, mipLevel2) * t;
        }
        // Nearest Neighbor
        else {
            Vector2f pos = getCoordinates(uv, mipLevel);
            res = pixels[(int)round(pos.x) + atlasSize.x * (int)round(pos.y)];
        }

        if constexpr(std::is_same_v<T, Vec3>) {
            return res.componentWiseMul(this->value);
        } else {
            return res * this->value;
        }
    }
};


#endif /* __TEXTUREFILTERING_H__ */
