#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Color.hpp>
#include <memory>

namespace beshray::_ {

class FrameBuffer {
  public:
    FrameBuffer(const unsigned w, const unsigned h) : w(w), h(h), pixelBuffer(new sf::Uint8[w * h * 4])
    {
        pixelBufferTex.create(w, h);
    };

    void present(sf::RenderWindow &window);
    void setPixel(const unsigned x, const unsigned y, const sf::Color c);
    sf::Color getPixel(const unsigned x, const unsigned y);

  private:
    const unsigned w = 0, h = 0;
    const std::unique_ptr<sf::Uint8> pixelBuffer;
    sf::Texture pixelBufferTex;

    inline unsigned index(const unsigned x, const unsigned y) const { return (y * (w * 4)) + (x * 4); }
};

} // namespace beshray::_