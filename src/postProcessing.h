#ifndef __POSTPROCESSING_H__
#define __POSTPROCESSING_H__

#include "color.h"
#include "data.h"
#include "texture.h"
#include "tinyTexture.h"
#include "vector3.h"
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

struct RenderedImage {
    std::vector<Color> data;
    sf::Vector2u size;
    RenderedImage(sf::Vector2u size, std::vector<Color> data) : data(data), size(size) {}
    RenderedImage(sf::Vector2u size) : data(size.x * size.y), size(size) {}
    RenderedImage(std::shared_ptr<RenderTarget> frame) : RenderedImage(frame->size, frame->framebuffer, frame->zBuffer) {}
    RenderedImage(sf::Vector2u size, std::vector<Color> colors, std::vector<float> depths) : data(size.x * size.y), size(size) {
        for (size_t i = 0; i < data.size(); i++) {
            Color color = colors[i];
            float depth = depths[i];
            data[i] = Color{color.r, color.g, color.b, depth};
        }
    }

    vector<Color> stripAlpha() {
        vector<Color> res(data.size());
        for (size_t i = 0; i < data.size(); i++) {
            Color c = data[i];
            res[i] = Color{c.r, c.g, c.b, 1};
        }
        return res;
    }

    RenderedImage clip(Color min, Color max) {
        RenderedImage img(size);
        for (size_t i = 0; i < data.size(); i++) {
            Color pixel = data[i];
            img.data[i] = Color{
                .r = std::clamp(pixel.r, min.r, max.r),
                .g = std::clamp(pixel.g, min.g, max.g),
                .b = std::clamp(pixel.b, min.b, max.b),
            };
        }
        return img;
    }

    shared_ptr<TinyImageTexture> tonemap(float whitePoint = 0) {
        if(whitePoint == 0) { // calculate biggest color
            auto it = std::max_element(
                data.begin(),
                data.end(),
                [](const auto& lhs, const auto& rhs) {
                    return lhs.luminance() < rhs.luminance();
                }
            );

            if (it != data.end()) {
                whitePoint = it->luminance();
            }
        }

        sf::Image img(size);
        for (unsigned int y = 0; y < size.y; y++) {
            for (unsigned int x = 0; x < size.x; x++) {
                Color pixel = data[y * size.x + x];
                Color res = pixel.reinhardtTonemap(whitePoint);
                res.a = 1;
                img.setPixel({x, y}, res);
            }
        }
        return std::make_shared<TinyImageTexture>(img, Color{1,1,1,1});
    }

    RenderedImage downscale(uint factor) {
        sf::Vector2u dstSize(size.x / factor, size.y / factor);

        RenderedImage res(dstSize);

        for (uint y = 0; y < dstSize.y; ++y) {
            for (uint x = 0; x < dstSize.x; ++x) {
                // Accumulate colors in the factor*factor block
                Color sum{};
                uint pixelsInBlock = 0;

                for (uint ky = 0; ky < factor; ky++) {
                    for (uint kx = 0; kx < factor; kx++) {
                        uint srcX = x * factor + kx;
                        uint srcY = y * factor + ky;

                        if (srcX < size.x && srcY < size.y) {
                            Color c = data[srcX + size.x * srcY];
                            sum += c;
                            pixelsInBlock++;
                        }
                    }
                }

                // Compute average color
                res.data[x + dstSize.x * y] = sum / (float)pixelsInBlock;
            }
        }

        return res;
    }

    RenderedImage blur(int kernelSize) {
        RenderedImage blurred(size);
        const float sigma = kernelSize * 0.5f;

        for (int y = 0; y < static_cast<int>(size.y); ++y) {
            for (int x = 0; x < static_cast<int>(size.x); ++x) {

                Color sum{};
                float weightSum = 0.0f;

                // radius == kernelSize
                for (int ky = -kernelSize; ky <= kernelSize; ++ky) {
                    for (int kx = -kernelSize; kx <= kernelSize; ++kx) {
                        int nx = x + kx;
                        int ny = y + ky;

                        if (nx >= 0 && ny >= 0 &&
                            nx < static_cast<int>(size.x) &&
                            ny < static_cast<int>(size.y)) {

                            // Gaussian weight for the current offset
                            float w = std::exp(-((kx * kx + ky * ky) /
                                                (2.0f * sigma * sigma)));

                            sum += data[nx + size.x * ny] * w;
                            weightSum += w;
                        }
                    }
                }

                blurred.data[x + blurred.size.x * y] = sum / weightSum;
            }
        }
        return blurred;
    }

    Color sample(sf::Vector2f uv) {
        float fx = uv.x * (size.x - 1);
        float fy = uv.y * (size.y - 1);

        // Integer part (floor) and fractional part for each axis
        int ix0 = static_cast<int>(std::floor(fx));
        int iy0 = static_cast<int>(std::floor(fy));
        float tx = fx - ix0;          // 0 ≤ tx < 1
        float ty = fy - iy0;          // 0 ≤ ty < 1

        int w = size.x, h = size.y;

        ix0 = std::clamp(ix0, 0, w - 2);   // - 2 because we need a+1
        iy0 = std::clamp(iy0, 0, h - 2);

        // Fetch the four neighboring pixels
        Color c00 = data[ix0 +     w * iy0];
        Color c10 = data[(ix0+1) + w * iy0];
        Color c01 = data[ix0 +     w * (iy0+1)];
        Color c11 = data[(ix0+1) + w * (iy0+1)];

        // Bilinear interpolation
        return lerp2d(c00, c10, c01, c11, tx, ty);
    }

    RenderedImage bloom(uint downscaleFactor, int kernelSize, float opacity) {
        RenderedImage downscaled = downscale(downscaleFactor);
        RenderedImage blurred = downscaled.blur(kernelSize);

        // 3. Blend blurred with source
        RenderedImage result(size);
        for (int y = 0; y < (int)(size.y); ++y) {
            for (int x = 0; x < (int)(size.x); ++x) {
                // map coordinates to the downscaled image
                Color srcPixel = data[x + size.x * y];
                
                // Convert coordinates to the resolution of `blurred`
                float u = ((float)(x) + 0.5f) / size.x;
                float v = ((float)(y) + 0.5f) / size.y;
                // blend using opacity (simple alpha blending)
                Color c = srcPixel + blurred.sample({u,v}) * opacity;

                result.data[x + result.size.x * y] = c;
            }
        }

        return result;
    }

    RenderedImage refract(shared_ptr<Texture<Vec3>> normalMap, Vector2f scale) {
        RenderedImage res(size);

        for (uint y = 0; y < size.y; ++y) {
            for (uint x = 0; x < size.x; ++x) {

                float u = ((float)(x) + 0.5f) / size.x;
                float v = ((float)(y) + 0.5f) / size.y;
                Vec3 normal = normalMap->sample({u,v}, {1.f/size.x, 0}, {0, 1.f/size.y});

                normal /= normal.z; // Normalize to Z=1 (NOT length=1)
                float depth = data[x + size.x * y].a;
                if(depth == INFINITY) depth = 10;
                int dx = normal.x * scale.x * depth;
                int dy = normal.y * scale.y * depth;
                uint newX = clamp<int>(x+dx, 0, size.x-1);
                uint newY = clamp<int>(y+dy, 0, size.y-1);

                res.data[x + size.x * y] = data[newX + size.x * newY];
            }
        }

        return res;
    }

    static TinyImageTexture downscale(TinyImageTexture src, uint factor) {
        sf::Vector2u srcSize = src.image.getSize();
        sf::Vector2u dstSize(srcSize.x / factor, srcSize.y / factor);

        sf::Image res(dstSize);

        for (uint y = 0; y < dstSize.y; ++y) {
            for (uint x = 0; x < dstSize.x; ++x) {
                // Accumulate colors in the factor*factor block
                unsigned long rSum = 0, gSum = 0, bSum = 0, aSum = 0;
                uint pixelsInBlock = 0;

                for (uint ky = 0; ky < factor; ky++) {
                    for (uint kx = 0; kx < factor; kx++) {
                        uint srcX = x * factor + kx;
                        uint srcY = y * factor + ky;

                        if (srcX < srcSize.x && srcY < srcSize.y) {
                            sf::Color c = src.image.getPixel({srcX, srcY});
                            rSum += c.r; gSum += c.g; bSum += c.b; aSum += c.a;
                            ++pixelsInBlock;
                        }
                    }
                }

                // Compute average color
                sf::Color avg(
                    (uint8_t)(rSum / pixelsInBlock),
                    (uint8_t)(gSum / pixelsInBlock),
                    (uint8_t)(bSum / pixelsInBlock),
                    (uint8_t)(aSum / pixelsInBlock)
                );
                res.setPixel({x, y}, avg);
            }
        }

        return TinyImageTexture(res, Color{1, 1, 1, 1});
    }
};

#endif /* __POSTPROCESSING_H__ */
