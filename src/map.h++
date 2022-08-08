#pragma once

#include "math/vector.h++"
#include "texture.h++"
#include <SFML/Graphics/Image.hpp>
#include <array>
#include <memory>
#include <vector>

namespace beshray {

class Map {
  public:
    unsigned w, h;
    Vec2i spawnLocation;

    struct Tile {
        Tile() = default;

        bool isSolid = false;

        enum class Side { East, North, West, South, Floor, Ceiling };
        std::array<Texture, 6> texture{nullptr};
    };

    Map() = default;
    Map(const std::string &filename);
    [[nodiscard]] const Tile &getTile(const unsigned x, const unsigned y) const { return tiles[x + y * w]; };
    [[nodiscard]] const Tile &getTile(const Vec2i pos) const { return tiles[pos.x + pos.y * w]; };

  private:
    std::vector<Tile> tiles;
};

} // namespace beshray