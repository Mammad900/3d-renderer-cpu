#ifndef __TINYTEXTURE_H__
#define __TINYTEXTURE_H__

#include "color.h"
#include "texture.h"
#include "textureFiltering.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/System/Vector2.hpp>

class TinyImageTexture : public SolidTexture<Color> {
  public:
    sf::Image image;
    TinyImageTexture() : SolidTexture<Color>(Color{}) {}
    TinyImageTexture(sf::Image &image, Color scale) : SolidTexture(scale), image(image) {}

    Color sample(Vector2f uv, Vector2f dUVdx, Vector2f dUVdy) {
        Vector2f pos = getCoordinates(uv);
        float decimalsX = pos.x - floor(pos.x);
        float decimalsY = pos.y - floor(pos.y);
        Color s1 = Color::fromSFColor(image.getPixel({(uint)floor(pos.x), (uint)floor(pos.y)}));
        Color s2 = Color::fromSFColor(image.getPixel({(uint)floor(pos.x), (uint)ceil(pos.y)}));
        Color s3 = Color::fromSFColor(image.getPixel({(uint)ceil(pos.x), (uint)floor(pos.y)}));
        Color s4 = Color::fromSFColor(image.getPixel({(uint)ceil(pos.x), (uint)ceil(pos.y)}));
        return lerp2d(s1, s2, s3, s4, decimalsY, decimalsX) * value;
    }

  private:
    Vector2f getCoordinates(Vector2f &uv) {
        float pos3x = clamp(uv.x, 0.0f, 1.0f) * (image.getSize().x - 1);
        float pos3y = clamp(uv.y, 0.0f, 1.0f) * (image.getSize().y - 1);
        return {pos3x, pos3y};
    }
};

#endif /* __TINYTEXTURE_H__ */
