#include "engine.h++"
#include "map.h++"
#include "math/vector.h++"

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <cmath>
#include <compare>
#include <cstring>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <thread>
#include <vector>

namespace beshray {

using Tile = Map::Tile;
using Side = Tile::Side;

// const sf::Color Engine::shadeSolidPixel(const Tile &tile, const Side side, const Vec2f sample, const float dist)
// const
// {
//     auto texture = tile.texture[(int)side];
//     if (!texture)
//         return sf::Color::Transparent;

//     int texX = int(sample.x * texture->getSize().x) & (texture->getSize().x - 1);
//     int texY = int(sample.y * texture->getSize().y) & (texture->getSize().y - 1);

//     auto pixel = texture->getPixel(texX, texY);

//     float fog = 1.0f - std::min(dist / 16.0f, 1.0f);

//     pixel.r = sf::Uint8(float(pixel.r) * fog);
//     pixel.g = sf::Uint8(float(pixel.g) * fog);
//     pixel.b = sf::Uint8(float(pixel.b) * fog);

//     return pixel;
// }

// const sf::Color Engine::shadeEntityPixel(const Entity &entity, const Vec2f sample, const float distance) const
// {
//     auto texture = entity.texture;
//     if (!texture)
//         return sf::Color::Transparent;

//     int texX = int(sample.x * texture->getSize().x) & (texture->getSize().x - 1);
//     int texY = int(sample.y * texture->getSize().y) & (texture->getSize().y - 1);

//     auto pixel = texture->getPixel(texX, texY);

//     float fog = 1.0f - std::min(distance / 16.0f, 1.0f);

//     pixel.r = sf::Uint8(float(pixel.r) * fog);
//     pixel.g = sf::Uint8(float(pixel.g) * fog);
//     pixel.b = sf::Uint8(float(pixel.b) * fog);

//     return pixel;
// }

void Engine::renderStripe(const int x)
{
    // x-coordinate in camera plane
    const float camX = 2 * x / float(width) - 1;

    // ray direction
    Vec2f rayDir = camera.dir + camX * camera.plane;

    const auto intersection = castRay(camera.pos, rayDir);
    if (!intersection) {
        return;
    }

    float wallDist = intersection->distance;

    // Calculate height of line to draw on screen
    int lineHeight = int(height / wallDist);

    // calculate lowest and highest pixel to fill in current stripe
    int drawStart = -(lineHeight / 2) + (height / 2) + camera.pitch + (camera.height / wallDist);
    if (drawStart < 0)
        drawStart = 0;

    int drawEnd = (lineHeight / 2) + (height / 2) + camera.pitch + (camera.height / wallDist);
    if (drawEnd >= height)
        drawEnd = height - 1;

    // calculate texture sample for y coordinate
    float texStep = 1.0f / lineHeight;
    float sampleY = (drawStart - camera.pitch - (camera.height / wallDist) - (height / 2) + (lineHeight / 2)) * texStep;

    for (int y = 0; y <= drawStart - 1; y++) {
        float currentDist = (height - (2.0 * camera.height)) / (height - 2.0 * (y - camera.pitch));

        Vec2f planePoint = camera.pos + rayDir * currentDist;
        auto planeTile = Vec2i(planePoint);

        const auto &tile = map.getTile(planeTile.x, planeTile.y);
        Vec2f sample = planePoint - static_cast<Vec2f>(planeTile);

        // auto pix = shadeSolidPixel(currentTile, Side::Ceiling, sample, currentDist);
        auto texture = tile.texture[(int)Side::Ceiling];
        if (!texture) {
            plotPixel(x, y, sf::Color{135, 206, 235});
            continue;
        }

        int texX = int(sample.x * texture->getSize().x) & (texture->getSize().x - 1);
        int texY = int(sample.y * texture->getSize().y) & (texture->getSize().y - 1);

        auto pixel = texture->getPixel(texX, texY);
        // plotPixel(x, y, pix);
        plotPixel(x, y, pixel);
        // plotPixel(x, y, pix);
    }

    for (int y = drawStart; y <= drawEnd + 1; y++) {
        sampleY += texStep;

        const auto &tile = map.getTile(intersection->tilePos);

        // auto pix = shadeSolidPixel(tile, intersection->side, {intersection->sampleX, sampleY}, wallDist);

        auto texture = tile.texture[(int)intersection->side];
        if (!texture) {
            plotPixel(x, y, sf::Color::Transparent);
            continue;
        }

        int texX = int(intersection->sampleX * texture->getSize().x) & (texture->getSize().x - 1);
        int texY = int(sampleY * texture->getSize().y) & (texture->getSize().y - 1);

        auto pixel = texture->getPixel(texX, texY);
        // plotPixel(x, y, pix);
        plotPixel(x, y, pixel);

        ZBuffer[x] = wallDist;
    }

    for (int y = drawEnd + 1; y <= height; y++) {
        float currentDist = (height + (2.0 * camera.height)) / (2.0 * (y - camera.pitch) - height);

        Vec2f planePoint = camera.pos + rayDir * currentDist;
        auto planeTile = Vec2i(planePoint);

        const auto &tile = map.getTile(planeTile.x, planeTile.y);
        Vec2f sample = planePoint - static_cast<Vec2f>(planeTile);

        // auto pix = shadeSolidPixel(currentTile, Side::Floor, sample, currentDist);
        auto texture = tile.texture[(int)Side::Floor];
        if (!texture) {
            plotPixel(x, y, sf::Color::Transparent);
            continue;
        }

        int texX = int(sample.x * texture->getSize().x) & (texture->getSize().x - 1);
        int texY = int(sample.y * texture->getSize().y) & (texture->getSize().y - 1);

        auto pixel = texture->getPixel(texX, texY);
        // plotPixel(x, y, pix);
        plotPixel(x, y, pixel);

        // plotPixel(x, y, pix);
    }
}

void Engine::render()
{
    // memset(pixelBuffer.get(), 0, width * height * 4 * sizeof(sf::Uint8));
    // pool.clear();

    // for (unsigned i = 0; i < 1; i++) {
    //     pool.emplace_back(&Engine::renderStripe, this, i);
    // }

    // for (auto &t : pool)
    //     t.join();

    static int factor = 0;
    const int step = 2;

    for (int x = 0 + factor; x < width; x += step)
        renderStripe(x);

    factor ^= 1;

    // std::vector<std::pair<float, int>> sprites(entities.size());

    // for (unsigned i = 0; i < entities.size(); i++) {
    //     sprites[i].first = (entities[i].pos - camera.pos).len();
    //     sprites[i].second = i;
    // }

    // std::ranges::sort(sprites, std::ranges::greater());

    // for (unsigned i = 0; i < entities.size(); i++) {
    //     const auto entity = entities[sprites[i].second];
    //     const float distance = sprites[i].first;

    //     // position relative to camera
    //     Vec2f entityPos = entity.pos - camera.pos;

    //     float invDet = 1.0f / (camera.plane.x * camera.dir.y - camera.dir.x * camera.plane.y);

    //     float transformX = invDet * (camera.dir.y * entityPos.x - camera.dir.x * entityPos.y);
    //     float transformY = invDet * (-camera.plane.y * entityPos.x + camera.plane.x * entityPos.y);

    //     int spriteScreenX = int((height) * (1 + transformX / transformY));

    //     constexpr float vDiv = 1.0f;
    //     constexpr float uDiv = 1.0f;
    //     constexpr float vMove = 0.0f;
    //     int vMoveScreen = int(vMove / transformY) + camera.pitch + camera.height / transformY;

    //     int spriteHeight = std::abs(int(height / (transformY))) / vDiv;

    //     int drawStartY = -spriteHeight / 2 + height / 2 + vMoveScreen;
    //     if (drawStartY < 0)
    //         drawStartY = 0;
    //     int drawEndY = spriteHeight / 2 + height / 2 + vMoveScreen;
    //     if (drawEndY >= height)
    //         drawEndY = height - 1;

    //     int spriteWidth = abs(int(height / (transformY))) / uDiv;
    //     int drawStartX = -spriteWidth / 2 + spriteScreenX;
    //     if (drawStartX < 0)
    //         drawStartX = 0;
    //     int drawEndX = spriteWidth / 2 + spriteScreenX;
    //     if (drawEndX >= width)
    //         drawEndX = width - 1;

    //     for (int stripe = drawStartX; stripe < drawEndX; stripe++) {
    //         int texX = int(256 * (stripe - (-spriteWidth / 2 + spriteScreenX)) * 64 / spriteWidth) / 256;

    //         if (transformY > 0 && stripe > 0 && stripe < width && transformY < ZBuffer[stripe]) {
    //             for (int y = drawStartY; y < drawEndY; y++) {
    //                 int d = (y - vMoveScreen) * 256 - height * 128 + spriteHeight * 128;

    //                 int texY = ((d * 64) / spriteHeight) / 256;

    //                 auto pix = shadeEntityPixel(entity, {static_cast<float>(texX), static_cast<float>(texY)},
    //                 distance);

    //                 if (pix.a != 0) {
    //                     plotPixel(stripe, y, pix);
    //                 }
    //             }
    //         }
    //     }
    // }
}

const std::optional<Engine::Intersection> Engine::castRay(const Vec2f origin, const Vec2f dir) const
{
    const float maxDistance = 100.0f;
    Intersection hit = {};

    Vec2f deltaDist = {std::abs(1 / dir.x), std::abs(1 / dir.y)};

    Vec2i mapCheck = {int(origin.x), int(origin.y)};
    Vec2f sideDist = {};
    Vec2i step = {};

    if (dir.x < 0) {
        step.x = -1;
        sideDist.x = (origin.x - float(mapCheck.x)) * deltaDist.x;
    }
    else {
        step.x = 1;
        sideDist.x = (float(mapCheck.x) + 1.0f - origin.x) * deltaDist.x;
    }

    if (dir.y < 0) {
        step.y = -1;
        sideDist.y = (origin.y - float(mapCheck.y)) * deltaDist.y;
    }
    else {
        step.y = 1;
        sideDist.y = (float(mapCheck.y) + 1.0f - origin.y) * deltaDist.y;
    }

    Vec2i hitTile = {};
    float distance = 0;

    bool isWallHit = false;
    bool isSide = false;

    while (!isWallHit && distance < maxDistance) {
        if (sideDist.x < sideDist.y) {
            sideDist.x += deltaDist.x;
            mapCheck.x += step.x;
            isSide = false;

            distance = (sideDist.x - deltaDist.x);
        }
        else {
            sideDist.y += deltaDist.y;
            mapCheck.y += step.y;
            isSide = true;

            distance = (sideDist.y - deltaDist.y);
        }

        hit.distance = distance;

        if (map.getTile(mapCheck.x, mapCheck.y).isSolid) {
            isWallHit = true;
            hitTile = mapCheck;
            hit.tilePos = mapCheck;
        }
    }

    if (!isSide) {
        hit.sampleX = origin.y + distance * dir.y;
    }
    else {
        hit.sampleX = origin.x + distance * dir.x;
    }
    hit.sampleX -= std::floor(hit.sampleX);

    if (isWallHit)
        return hit;

    return {};
}

void Engine::present(sf::RenderWindow &window)
{
    pixelBufferTex.update(pixelBuffer.get());
    sf::Sprite s{pixelBufferTex};
    window.draw(s);
}

inline void Engine::plotPixel(const unsigned x, const unsigned y, const sf::Color c) const
{
    pixelBuffer.get()[(y * (width * 4)) + (x * 4)] = c.r;
    pixelBuffer.get()[((y * (width * 4)) + (x * 4)) + 1] = c.g;
    pixelBuffer.get()[((y * (width * 4)) + (x * 4)) + 2] = c.b;
    pixelBuffer.get()[((y * (width * 4)) + (x * 4)) + 3] = c.a;
}

} // namespace beshray