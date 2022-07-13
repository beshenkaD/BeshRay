#pragma once

#include <SFML/Graphics/Image.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <memory>
#include <vector>

class Tile {
   private:
    Tile();

   public:
    const bool isWall;

    std::shared_ptr<sf::Image> wall;
    std::shared_ptr<sf::Image> ceiling;
    std::shared_ptr<sf::Image> floor;

    Tile(bool isWall, std::shared_ptr<sf::Image> wall, std::shared_ptr<sf::Image> ceiling, std::shared_ptr<sf::Image> floor)
        : isWall(isWall), wall(wall), ceiling(ceiling), floor(floor){};
};

class Map {
   private:
    Map();

    std::vector<Tile> tiles;

   public:
    size_t w, h;
    sf::Vector2f spawnLocation;

    Map(const std::string& filename);
    Tile& getTile(const size_t x, const size_t y);
};
