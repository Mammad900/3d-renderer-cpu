#include "textureFiltering.h"

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