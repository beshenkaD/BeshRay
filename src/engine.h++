#pragma once

#include "map.h++"
#include "math/vector.h++"
#include "memory"
#include "texture.h++"

#include <SFML/Graphics.hpp>
#include <optional>
#include <vector>

namespace beshray {

struct Camera {
    Vec2f pos = {0, 0};
    Vec2f dir = {-1, 0};
    Vec2f plane = {0, 1};

    float pitch = 0;
    float height = 0;

    void move(float speed) { pos += dir * speed; };
    void tilt(float speed) { pitch += speed; };
    void lift(float speed) { height += speed; };
    void rotate(float angle)
    {
        dir = dir.rotate(angle);
        plane = plane.rotate(angle);
    };
};

struct Entity {
    Vec2f pos = {0, 0};
    float w = 1, h = 1;
    Texture texture = nullptr;
};

class Engine {
  public:
    const int width, height;

    std::vector<Entity> entities;

    Engine(const int w = 960, const int h = 540) : width(w), height(h), pixelBuffer(new sf::Uint8[w * h * 4])
    {
        pixelBufferTex.create(w, h);
        ZBuffer.reserve(w * h);
    };

    void render();
    void present(sf::RenderWindow &window);

    void setInterlacing(bool enable)
    {
        interlacing = enable;
        if (!enable)
            step = 1;
    }

    Camera camera;
    Map map;

  private:
    std::unique_ptr<sf::Uint8> pixelBuffer;
    sf::Texture pixelBufferTex;

    std::vector<float> ZBuffer;

    bool interlacing = true;
    int factor = 0;
    int step = 2;

    struct Intersection {
        Vec2i tilePos = {0, 0};
        Vec2f hitPos = {0, 0};
        float distance = 0;
        float sampleX = 0;
        Map::Tile::Side side = Map::Tile::Side::East;
    };

    inline void plotPixel(const unsigned x, const unsigned y, const sf::Color c) const;
    void renderWorld(const int from, const int to);
    const std::optional<Intersection> castRay(const Vec2f origin, const Vec2f dir) const;
};

} // namespace beshray