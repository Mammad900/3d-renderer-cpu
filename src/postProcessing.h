#ifndef __POSTPROCESSING_H__
#define __POSTPROCESSING_H__

#include "color.h"
#include "data.h"
#include "tinyTexture.h"
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <cstdint>
#include <memory>
#include <vector>

// Gaussian weights for a 3x3 kernel (σ ≈ 0.8)
static constexpr float gaussian[3][3] = {
    {0.0751136f, 0.1238414f, 0.0751136f},
    {0.1238414f, 0.2041799f, 0.1238414f},
    {0.0751136f, 0.1238414f, 0.0751136f}
};


struct RenderedImage {
    std::vector<Color> data;
    sf::Vector2u size;
    RenderedImage(sf::Vector2u size, std::vector<Color> data) : data(data), size(size) {}
    RenderedImage(sf::Vector2u size) : data(size.x * size.y), size(size) {}
    RenderedImage(std::shared_ptr<RenderTarget> frame) : data(frame->framebuffer), size(frame->size) {}

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
                img.setPixel({x, y}, pixel.reinhardtTonemap(whitePoint));
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

    RenderedImage bloom(uint downscaleFactor, int kernelSize, float opacity) {
        RenderedImage downscaled = downscale(downscaleFactor);

        // 2. Blur the downscaled image – dynamic Gaussian kernel
        RenderedImage blurred(downscaled.size);
        const float sigma = kernelSize * 0.5f;          // simple heuristic

        for (int y = 0; y < static_cast<int>(downscaled.size.y); ++y) {
            for (int x = 0; x < static_cast<int>(downscaled.size.x); ++x) {

                Color sum{};
                float weightSum = 0.0f;

                // radius == kernelSize
                for (int ky = -kernelSize; ky <= kernelSize; ++ky) {
                    for (int kx = -kernelSize; kx <= kernelSize; ++kx) {
                        int nx = x + kx;
                        int ny = y + ky;

                        if (nx >= 0 && ny >= 0 &&
                            nx < static_cast<int>(downscaled.size.x) &&
                            ny < static_cast<int>(downscaled.size.y)) {

                            // Gaussian weight for the current offset
                            float w = std::exp(-((kx * kx + ky * ky) /
                                                (2.0f * sigma * sigma)));

                            sum += downscaled.data[nx + downscaled.size.x * ny] * w;
                            weightSum += w;
                        }
                    }
                }

                blurred.data[x + blurred.size.x * y] = sum / weightSum;
            }
        }

        // 3. Blend blurred with source
        RenderedImage result(size);
        for (int y = 0; y < (int)(size.y); ++y) {
            for (int x = 0; x < (int)(size.x); ++x) {
                // map coordinates to the downscaled image
                Color srcPixel = data[x + size.x * y];
                
                // Convert coordinates to the resolution of `blurred`
                float fx = ((float)(x) + 0.5f) * (blurred.size.x-1) / size.x;
                float fy = ((float)(y) + 0.5f) * (blurred.size.y-1) / size.y;

                // Integer part (floor) and fractional part for each axis
                int ix0 = static_cast<int>(std::floor(fx));
                int iy0 = static_cast<int>(std::floor(fy));
                float tx = fx - ix0;          // 0 ≤ tx < 1
                float ty = fy - iy0;          // 0 ≤ ty < 1

                // Clamp to image bounds (ensure we never read outside the buffer)
                int w = blurred.size.x;
                int h = blurred.size.y;

                ix0 = std::clamp(ix0, 0, w - 2);   // - 2 because we need a+1
                iy0 = std::clamp(iy0, 0, h - 2);

                // Fetch the four neighboring pixels
                Color c00 = blurred.data[ix0 +     w * iy0];
                Color c10 = blurred.data[(ix0+1) + w * iy0];
                Color c01 = blurred.data[ix0 +     w * (iy0+1)];
                Color c11 = blurred.data[(ix0+1) + w * (iy0+1)];

                // Bilinear interpolation
                Color bloomPixel = lerp2d(c00, c10, c01, c11, tx, ty);

                // blend using opacity (simple alpha blending)
                Color c = srcPixel + bloomPixel * opacity;

                result.data[x + result.size.x * y] = c;
            }
        }

        return result;
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
