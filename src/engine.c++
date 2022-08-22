#include "engine.h++"
#include "map.h++"
#include "math/vector.h++"

#include <chrono>
#include <optional>

namespace beshray {

using Tile = Map::Tile;
using Side = Tile::Side;

inline void Engine::renderWorld()
{
    for (unsigned x = 0 + factor; x < width; x += step) {
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
        int drawStart = std::max(int(-(lineHeight / 2) + (height / 2) + camera.pitch + (camera.height / wallDist)), 0);
        int drawEnd = std::min(int((lineHeight / 2) + (height / 2) + camera.pitch + (camera.height / wallDist)), int(height));

        for (int y = 0; y <= drawStart - 1; y++) {
            float currentDist = (height - (2.0 * camera.height)) / (height - 2.0 * (y - camera.pitch));

            Vec2f planePoint = camera.pos + rayDir * currentDist;
            auto planeTile = Vec2i(planePoint);

            const auto &currentTile = map.getTile(planeTile.x, planeTile.y);

            auto texture = currentTile.texture[(int)Side::Ceiling];
            if (!texture) {
                framebuffer.setPixel(x, y, sf::Color(135, 206, 235));
                continue;
            }

            Vec2f sample = planePoint - static_cast<Vec2f>(planeTile);

            int texX = int(sample.x * texture->getSize().x) & (texture->getSize().x - 1);
            int texY = int(sample.y * texture->getSize().y) & (texture->getSize().y - 1);

            auto pixel = texture->getPixel(texX, texY);
            framebuffer.setPixel(x, y, pixel);
        }

        // calculate texture sample for y coordinate
        float texStep = 1.0f / lineHeight;
        float sampleY = (drawStart - camera.pitch - (camera.height / wallDist) - (height / 2) + (lineHeight / 2)) * texStep;

        for (int y = drawStart; y <= drawEnd + 1; y++) {
            sampleY += texStep;

            int texX = int(intersection->sampleX * texture->getSize().x) & (texture->getSize().x - 1);
            int texY = int(sampleY * texture->getSize().y) & (texture->getSize().y - 1);

            const auto pixel = texture->getPixel(texX, texY);
            framebuffer.setPixel(x, y, pixel);
        }

        ZBuffer.set(x, wallDist);

        for (unsigned y = drawEnd + 1; y <= height; y++) {
            float currentDist = (height + (2.0 * camera.height)) / (2.0 * (y - camera.pitch) - height);

            Vec2f planePoint = camera.pos + rayDir * currentDist;
            auto planeTile = Vec2i(planePoint);

            const auto &currentTile = map.getTile(planeTile.x, planeTile.y);

            auto texture = currentTile.texture[(int)Side::Floor];
            if (!texture) {
                framebuffer.setPixel(x, y, sf::Color::Black);
                continue;
            }

            Vec2f sample = planePoint - static_cast<Vec2f>(planeTile);

            int texX = int(sample.x * texture->getSize().x) & (texture->getSize().x - 1);
            int texY = int(sample.y * texture->getSize().y) & (texture->getSize().y - 1);

            auto pixel = texture->getPixel(texX, texY);
            framebuffer.setPixel(x, y, pixel);
        }
    }
}

inline void Engine::sortSprites()
{
    for (auto &sprite : sprites) {
        if (!sprite.visible) {
            continue;
        }

        // position relative to camera
        Vec2f entityPos = sprite.pos - camera.pos;

        float invDet = 1.0f / (camera.plane.x * camera.dir.y - camera.dir.x * camera.plane.y);

        float transformX = invDet * (camera.dir.y * entityPos.x - camera.dir.x * entityPos.y);
        float transformY = invDet * (-camera.plane.y * entityPos.x + camera.plane.x * entityPos.y); // depth

        sprite.relativePos = entityPos;
        sprite.transform = Vec2f{transformX, transformY};
    }

    std::sort(sprites.begin(), sprites.end(),
              [](const Sprite &a, const Sprite &b) -> bool { return a.transform.y > b.transform.y; });
}

// TODO: optimize
inline void Engine::renderSprites()
{
    for (const auto &sprite : sprites) {
        float transformX = sprite.transform.x;
        float transformY = sprite.transform.y;

        if (transformY <= 0.5) {
            continue;
        }

        int vMoveScreen = camera.pitch + camera.height / transformY;
        int spriteScreenX = int((width / 2) * (1 + transformX / transformY));

        int spriteHeight = std::abs(int(height / (transformY)));
        int spriteWidth = spriteHeight;

        int drawStartY = std::max(int(-spriteHeight / 2 + height / 2 + vMoveScreen), 0);
        int drawEndY = std::min(spriteHeight / 2 + height / 2 + vMoveScreen, height);

        int drawStartX = std::max(-spriteWidth / 2 + spriteScreenX, 0);
        int drawEndX = std::min(spriteWidth / 2 + spriteScreenX, int(width));

        if (interlacing) {
            if ((drawStartX % 2 == 0 && factor == 1) || (drawStartX % 2 != 0 && factor == 0)) {
                drawStartX++;
            }
        }

        for (int x = drawStartX; x < drawEndX; x += step) {
            if (ZBuffer.get(x) < transformY) {
                continue;
            }

            for (int y = drawStartY; y < drawEndY; y++) {
                int texX = int(256 * (x - (-spriteWidth / 2 + spriteScreenX)) * 64 / spriteWidth) / 256;
                int d = (y - vMoveScreen) * 256 - height * 128 + spriteHeight * 128;
                int texY = ((d * 64) / spriteHeight) / 256;

                auto pixel = sprite.texture->getPixel(texX, texY);

                if (pixel.a == 255) {
                    framebuffer.setPixel(x, y, pixel);
                }
            }
        }
    }
}

void Engine::render()
{
    if (interlacing)
        factor ^= 1;

    renderWorld();
    sortSprites();
    renderSprites();
}

// TODO:
// 1. add intesection point
// 2. detect wall side
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

            if (isSide) {
                hit.sampleX = origin.x + hit.distance * dir.x;
            }
            else {
                hit.sampleX = origin.y + hit.distance * dir.y;
            }
            hit.sampleX -= std::floor(hit.sampleX);
        }
    }

    return isWallHit ? hit : std::optional<Intersection>{};
}

int Engine::start(const std::string &appName)
{
    if (!onCreate()) {
        return 1;
    }

    sf::RenderWindow window(sf::VideoMode(width, height), appName, sf::Style::Default);

    std::chrono::steady_clock::time_point previous;

    while (window.isOpen()) {
        auto now = std::chrono::steady_clock::now();
        auto deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(now - previous).count() / 1'000'000.0f;

        sf::Event event{};

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        window.clear();

        onUpdate(deltaTime);

        framebuffer.present(window);
        window.display();

        previous = now;
    }

    return 0;
}

} // namespace beshray