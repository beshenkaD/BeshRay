#pragma once

#include "camera.h++"
#include "framebuffer.h++"
#include "map.h++"

#include <optional>

namespace beshray {
namespace _ {

struct ZBuffer {
  public:
    ZBuffer(const unsigned w) : buffer(new float[w]){};

    inline void set(const int x, const float distance) { buffer[x] = distance; }
    inline float get(const int x) { return buffer[x]; }

  private:
    const std::unique_ptr<float[]> buffer; // NOLINT
};

} // namespace _

class Sprite {
  public:
    Sprite() = default;
    Sprite(Vec2f p, Texture t) : pos(p), texture(std::move(t)){};

    Vec2f pos = {0, 0};
    Texture texture = nullptr;
    bool visible = true;

  private:
    Vec2f relativePos{0, 0};
    Vec2f transform{0, 0};

    friend class Engine;
};

// TODO:
// 1. add framebuffer class
// 2. add config
class Engine {
  public:
    const unsigned width, height;

    std::vector<Sprite> sprites;

    Camera camera;

    Map map;

  public:
    Engine(const unsigned w, const unsigned h) : width(w), height(h){};

    void setInterlacing(bool enable)
    {
        interlacing = enable;
        if (!enable)
            step = 1;
    }

    void render();
    int start(const std::string &appName);

    virtual bool onCreate() = 0;
    virtual void onUpdate(float deltaTime) = 0;

  private:
    _::FrameBuffer framebuffer{width, height};
    _::ZBuffer ZBuffer{width};

    bool interlacing = false;
    int factor = 0;
    int step = 1;

    struct Intersection {
        Vec2i tilePos = {0, 0};
        Vec2f hitPos = {0, 0};
        float distance = 0;
        float sampleX = 0;
        Map::Tile::Side side = Map::Tile::Side::East;
    };

  private:
    inline void renderWorld();
    inline void sortSprites();
    inline void renderSprites();
    [[nodiscard]] const std::optional<Intersection> castRay(const Vec2f origin, const Vec2f dir) const;
};

} // namespace beshray