#include "map.h++"
#include "json.h++"
#include <SFML/Graphics/Image.hpp>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

namespace beshray {

using namespace nlohmann;

Map::Map(const std::string &filename)
{
    std::ifstream i(filename);
    json j;

    i >> j;

    w = j["properties"]["w"];
    h = j["properties"]["h"];
    spawnLocation = {j["properties"]["spawn"][0], j["properties"]["spawn"][1]};

    // TODO: use os-indpendent way to concatenate paths
    std::vector<std::shared_ptr<sf::Image>> textures;
    for (auto &rawPath : j["textures"]) {
        sf::Image tex;

        auto path = "../resources/" + rawPath.get<std::string>();

        if (!tex.loadFromFile(path)) {
            abort();
        }

        textures.push_back(std::make_shared<sf::Image>(tex));
    }

    auto walls = j["map"]["walls"].get<std::vector<int>>();
    auto ceiling = j["map"]["ceiling"].get<std::vector<int>>();
    auto floor = j["map"]["floor"].get<std::vector<int>>();

    if (walls.size() == ceiling.size() && ceiling.size() == floor.size()) {
        for (size_t i = 0; i < walls.size(); i++) {
            bool wall = (walls[i] < 0) ? false : true;

            auto wallTex = (wall) ? textures[walls[i]] : nullptr;
            auto ceilingTex = (ceiling[i] < 0) ? nullptr : textures[ceiling[i]];
            auto floorTex = (floor[i] < 0) ? nullptr : textures[floor[i]];

            Tile t{};
            t.texture.fill(wallTex);
            t.texture[(int)Tile::Side::Ceiling] = ceilingTex;
            t.texture[(int)Tile::Side::Floor] = floorTex;

            t.isSolid = wall;

            tiles.push_back(t);
        }
    }
    else {
        abort();
    }

    i.close();
}

} // namespace beshray