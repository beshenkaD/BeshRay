#include "engine.h++"
#include "map.h++"
#include "math/vector.h++"

#include <SFML/Graphics.hpp>
#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
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

void Engine::renderWorld(const int from, const int to)
{
    for (int x = from + factor; x < to; x += step) {
        // x-coordinate in camera plane
        float camX = 2 * x / float(width) - 1;

        // ray direction
        Vec2f rayDir = camera.dir + camX * camera.plane;

        auto intersection = castRay(camera.pos, rayDir);
        if (!intersection) {
            continue;
        }

        const auto &tile = map.getTile(intersection->tilePos);
        const auto texture = tile.texture[(int)intersection->side];

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

        for (int y = 0; y <= drawStart - 1; y++) {
            float currentDist = (height - (2.0 * camera.height)) / (height - 2.0 * (y - camera.pitch));

            Vec2f planePoint = camera.pos + rayDir * currentDist;
            auto planeTile = Vec2i(planePoint);

            const auto &currentTile = map.getTile(planeTile.x, planeTile.y);

            auto texture = currentTile.texture[(int)Side::Ceiling];
            if (!texture) {
                plotPixel(x, y, sf::Color{135, 206, 235});
                continue;
            }

            Vec2f sample = planePoint - static_cast<Vec2f>(planeTile);

            int texX = int(sample.x * texture->getSize().x) & (texture->getSize().x - 1);
            int texY = int(sample.y * texture->getSize().y) & (texture->getSize().y - 1);

            auto pixel = texture->getPixel(texX, texY);
            plotPixel(x, y, pixel);
        }

        // calculate texture sample for y coordinate
        float texStep = 1.0f / lineHeight;
        float sampleY =
            (drawStart - camera.pitch - (camera.height / wallDist) - (height / 2) + (lineHeight / 2)) * texStep;

        for (int y = drawStart; y <= drawEnd + 1; y++) {
            sampleY += texStep;

            int texX = int(intersection->sampleX * texture->getSize().x) & (texture->getSize().x - 1);
            int texY = int(sampleY * texture->getSize().y) & (texture->getSize().y - 1);

            const auto pixel = texture->getPixel(texX, texY);
            plotPixel(x, y, pixel);
            ZBuffer[y * width + x] = wallDist;
        }

        for (int y = drawEnd + 1; y <= height; y++) {
            float currentDist = (height + (2.0 * camera.height)) / (2.0 * (y - camera.pitch) - height);

            Vec2f planePoint = camera.pos + rayDir * currentDist;
            auto planeTile = Vec2i(planePoint);

            const auto &currentTile = map.getTile(planeTile.x, planeTile.y);

            auto texture = currentTile.texture[(int)Side::Floor];
            // if (!texture) {
            //     continue;
            // }

            Vec2f sample = planePoint - static_cast<Vec2f>(planeTile);

            int texX = int(sample.x * texture->getSize().x) & (texture->getSize().x - 1);
            int texY = int(sample.y * texture->getSize().y) & (texture->getSize().y - 1);

            auto pixel = texture->getPixel(texX, texY);
            plotPixel(x, y, pixel);
        }
    }

    for (const auto &entity : entities) {
        // position relative to camera
        Vec2f entityPos = entity.pos - camera.pos;

        float invDet = 1.0f / (camera.plane.x * camera.dir.y - camera.dir.x * camera.plane.y);

        float transformX = invDet * (camera.dir.y * entityPos.x - camera.dir.x * entityPos.y);
        float transformY = invDet * (-camera.plane.y * entityPos.x + camera.plane.x * entityPos.y); // z

        if (transformY <= 0.5) {
            continue;
        }

        int spriteScreenX = int((width / 2) * (1 + transformX / transformY));

        int vMoveScreen = int(transformY) + camera.pitch + camera.height / transformY;

        int spriteHeight = std::abs(int(height / (transformY)));

        int drawStartY = -spriteHeight / 2 + height / 2 + vMoveScreen;
        if (drawStartY < 0)
            drawStartY = 0;

        int drawEndY = spriteHeight / 2 + height / 2 + vMoveScreen;
        if (drawEndY >= height)
            drawEndY = height - 1;

        int spriteWidth = abs(int(height / (transformY)));
        int drawStartX = -spriteWidth / 2 + spriteScreenX;
        if (drawStartX < 0)
            drawStartX = 0;

        int drawEndX = spriteWidth / 2 + spriteScreenX;
        if (drawEndX >= width)
            drawEndX = width - 1;

        if (interlacing) {
            if ((drawStartX % 2 == 0 && factor == 1) || (drawStartX % 2 != 0 && factor == 0)) {
                drawStartX++;
            }
        }

        for (int x = drawStartX; x < drawEndX; x += step) {
            if (transformY > 0 && x > 0 && x < width) {
                for (int y = drawStartY; y < drawEndY; y++) {
                    if (transformY > ZBuffer[y * width + x]) {
                        break;
                    }

                    int texX = int(256 * (x - (-spriteWidth / 2 + spriteScreenX)) * 64 / spriteWidth) / 256;
                    int d = (y - vMoveScreen) * 256 - height * 128 + spriteHeight * 128;
                    int texY = ((d * 64) / spriteHeight) / 256;

                    auto pixel = entity.texture->getPixel(texX, texY);

                    if (pixel.a != 0) {
                        plotPixel(x, y, pixel);
                        ZBuffer[y * width + x] = transformY;
                    }
                }
            }
        }
    }
}

void Engine::render()
{
    for (int i = 0; i < width * height; i++) {
        ZBuffer[i] = std::numeric_limits<float>::infinity();
    }

    if (interlacing)
        factor ^= 1;

    renderWorld(0, width);

    // for (const auto &task : tasks) {
    //     pool.push_task(&Engine::renderWorld, this, task.from, task.to);
    // }

    // pool.wait_for_tasks();

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

    //                 // auto pix = shadeEntityPixel(entity, {static_cast<float>(texX), static_cast<float>(texY)},
    //                 // distance);

    //                 // texX = int(texX * entity.texture->getSize().x) & (entity.texture->getSize().x - 1);
    //                 // texY = int(texY * entity.texture->getSize().y) & (entity.texture->getSize().y - 1);

    //                 auto pixel = entity.texture->getPixel(texX, texY);

    //                 if (pixel.a != 0) {
    //                     plotPixel(stripe, y, pixel);
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

    bool isWallHit = false;
    bool isSide = false;

    while (!isWallHit && hit.distance < maxDistance) {
        if (sideDist.x < sideDist.y) {
            sideDist.x += deltaDist.x;
            mapCheck.x += step.x;
            isSide = false;
        }
        else {
            sideDist.y += deltaDist.y;
            mapCheck.y += step.y;
            isSide = true;
        }

        if (map.getTile(mapCheck.x, mapCheck.y).isSolid) {
            isWallHit = true;
            hitTile = mapCheck;
            hit.tilePos = mapCheck;

            hit.distance = isSide ? (sideDist.y - deltaDist.y) : (sideDist.x - deltaDist.x);

            if (!isSide) {
                hit.sampleX = origin.y + hit.distance * dir.y;
            }
            else {
                hit.sampleX = origin.x + hit.distance * dir.x;
            }
            hit.sampleX -= std::floor(hit.sampleX);
        }
    }

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