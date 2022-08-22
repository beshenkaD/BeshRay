#include "framebuffer.h++"
#include <SFML/Graphics/Color.hpp>

namespace beshray::_ {

void FrameBuffer::present(sf::RenderWindow &window)
{
    pixelBufferTex.update(pixelBuffer.get());
    sf::Sprite s{pixelBufferTex};
    window.draw(s);
}

void FrameBuffer::setPixel(const unsigned x, const unsigned y, const sf::Color c)
{
    const unsigned i = index(x, y);
    auto buffer = pixelBuffer.get();

    buffer[i + 0] = c.r;
    buffer[i + 1] = c.g;
    buffer[i + 2] = c.b;
    buffer[i + 3] = c.a;
}

sf::Color FrameBuffer::getPixel(const unsigned x, const unsigned y)
{
    const unsigned i = index(x, y);
    auto buffer = pixelBuffer.get();

    return sf::Color{
        buffer[i + 0],
        buffer[i + 1],
        buffer[i + 2],
        buffer[i + 3],
    };
}

} // namespace beshray::_