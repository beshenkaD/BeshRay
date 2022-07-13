#include <SFML/Config.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Vector2.hpp>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <numbers>
#include <string>
#include <unordered_map>
#include <vector>
#include "map.h++"

// const size_t map_w = 16;
// const size_t map_h = 16;

// // clang-format off
// const int map[map_w*map_h] = {
//     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
//     1, 0, 0, 0, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 1,
//     1, 0, 0, 0, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 1,
//     1, 0, 0, 0, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 1,
//     1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
//     1, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 1,
//     1, 0, 0, 4, 0, 0, 4, 2, 2, 0, 0, 0, 0, 0, 0, 1,
//     1, 0, 0, 4, 0, 0, 2, 0, 2, 0, 0, 0, 0, 0, 0, 1,
//     1, 0, 0, 4, 0, 0, 2, 2, 2, 0, 0, 0, 0, 0, 0, 1,
//     1, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
//     1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
//     1, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 1,
//     1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
//     1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
//     1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
//     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
// };
// // clang-format on

const auto W = 1280;
const auto H = 720;

// const auto TEX_SIZE = 512;
// const auto TEX_WALL = 128;

// enum class WallTexture {
//     Smiley,
//     Red,
//     Bush,
//     Sky,
//     Pink,
//     Wallpaper,
//     Dirt,
//     Exit,
// };

// const std::unordered_map<int, WallTexture> textureTypes{
//     {0, WallTexture::Pink}, {1, WallTexture::Dirt}, {2, WallTexture::Wallpaper}, {3, WallTexture::Bush},
//     {4, WallTexture::Sky},  {5, WallTexture::Red},  {6, WallTexture::Smiley},    {7, WallTexture::Exit},
// };

struct Player {
    sf::Vector2f position;
    sf::Vector2f direction;

    //    private:
    sf::Vector2f camera;  // camera plane (line)

    // float height = 0.666;

    Player() : position(sf::Vector2f(22, 12)), direction(sf::Vector2f(-1, 0)), camera(sf::Vector2f(0, 1)){};

    void moveForward(float speed);
    void moveBackwards(float speed);
    void rotateLeft(float speed);
    void rotateRight(float speed);

   private:
    sf::Vector2f rotate(sf::Vector2f& v, float angle);
};

void Player::moveForward(float speed) {
    position += speed * direction;
}

void Player::moveBackwards(float speed) {
    position -= speed * direction;
}

sf::Vector2f Player::rotate(sf::Vector2f& v, float angle) {
    // clang-format off
    return sf::Vector2f(
        v.x * std::cos(angle) - v.y * std::sin(angle), 
        v.x * std::sin(angle) + v.y * std::cos(angle)
    );
    // clang-format on
}

void Player::rotateLeft(float speed) {
    direction = rotate(direction, speed);
    camera = rotate(camera, speed);
}

void Player::rotateRight(float speed) {
    speed = -speed;

    direction = rotate(direction, speed);
    camera = rotate(camera, speed);
}

class Raycaster {
    sf::VertexArray buffer;

    std::vector<float> distTable;

    void calculateDistTable();

   public:
    Raycaster() {
        buffer = sf::VertexArray(sf::PrimitiveType::Points, W * H);

        calculateDistTable();
    };

    void renderFrame(sf::RenderWindow& window, Player& player, Map& map);
};

void Raycaster::calculateDistTable() {
    for (int y = 0; y < H; y++) {
        distTable.push_back(H / (2.0 * y - H));
    }
}

void Raycaster::renderFrame(sf::RenderWindow& window, Player& player, Map& map) {
    buffer.resize(0);

    // draw sky
    // for (int x = 0; x < W; x++) {
    // }

    for (int x = 0; x < W; x++) {
        // ray position and direction
        float camX = 2 * x / float(W) - 1;  // x-coordinate in camera plane(line). 1 - top right, 0 - center, -1 - top left

        // ray direction
        auto rayDir = player.direction + camX * player.camera;

        // which box of the map we're in
        auto map_ = sf::Vector2i(static_cast<int>(player.position.x), static_cast<int>(player.position.y));

        // length of ray from current position to next x or y-side
        auto sideDist = sf::Vector2f();

        // length of ray from one x or y-side to next x or y-side
        auto deltaDist = sf::Vector2f(std::abs(1 / rayDir.x), std::abs(1 / rayDir.y));

        // what direction to step in x or y-direction (either +1 or -1)
        auto step = sf::Vector2i();

        bool hit = false;   // was there a wall hit?
        bool side = false;  // was a NS or a EW wall hit?

        // calculate step and initial sideDist
        if (rayDir.x < 0) {
            step.x = -1;
            sideDist.x = (player.position.x - map_.x) * deltaDist.x;
        } else {
            step.x = 1;
            sideDist.x = (map_.x + 1.0 - player.position.x) * deltaDist.x;
        }
        if (rayDir.y < 0) {
            step.y = -1;
            sideDist.y = (player.position.y - map_.y) * deltaDist.y;
        } else {
            step.y = 1;
            sideDist.y = (map_.y + 1.0 - player.position.y) * deltaDist.y;
        }

        // perform DDA
        while (hit == false) {
            // jump to next map square, either in x-direction, or in y-direction
            if (sideDist.x < sideDist.y) {
                sideDist.x += deltaDist.x;
                map_.x += step.x;
                side = false;
            } else {
                sideDist.y += deltaDist.y;
                map_.y += step.y;
                side = true;
            }

            // Check if ray has hit a wall
            if (map.getTile(map_.x, map_.y).isWall)
                hit = true;
        }

        // Calculate distance projected on camera direction (Euclidean distance
        // would give fisheye effect!)
        float perpWallDist;
        if (side == 0)
            perpWallDist = (sideDist.x - deltaDist.x);
        else
            perpWallDist = (sideDist.y - deltaDist.y);

        // Calculate height of line to draw on screen
        int lineHeight = static_cast<int>(H / perpWallDist);

        // calculate lowest and highest pixel to fill in current stripe
        int drawEnd = lineHeight / 2 + H / 2;
        if (drawEnd >= H)
            drawEnd = H - 1;

        int drawStart = H / 2 - lineHeight / 2;
        if (drawStart < 0)
            drawStart = 0;

        // RENDERING WALLS
        // textures
        float wallX;  // where the wall was hit
        if (!side) {
            wallX = player.position.y + perpWallDist * rayDir.y;
        } else {
            wallX = player.position.x + perpWallDist * rayDir.x;
        }
        wallX -= std::floor(wallX);

        auto wallTexture = map.getTile(map_.x, map_.y).wall;

        // x coordinate of the texture
        int texX = static_cast<int>(wallX * wallTexture->getSize().x);
        if ((!side && rayDir.x <= 0) || (side && rayDir.y >= 0)) {
            texX = wallTexture->getSize().x - texX - 1;
        }

        float texStep = 1.0f * wallTexture->getSize().y / lineHeight;
        float texPos = (drawStart - H / 2 + lineHeight / 2) * texStep;

        for (int y = drawStart; y < drawEnd; y++) {
            int texY = (int)texPos & (wallTexture->getSize().y - 1);
            texPos += texStep;

            buffer.append(sf::Vertex(sf::Vector2f(x, y), wallTexture->getPixel(texX, texY)));
        }

        // shading
        auto color_ = sf::Color::White;
        if (side) {
            color_.r >>= 1;
            color_.g >>= 1;
            color_.b >>= 1;
        }

        // lines.append(sf::Vertex(sf::Vector2f(x, drawStart), color_, sf::Vector2f(texCoords.x, texCoords.y)));
        // lines.append(sf::Vertex(sf::Vector2f(x, drawEnd), color_, sf::Vector2f(texCoords.x, texCoords.y + TEX_WALL)));

        // // FLOOR
        float floorXWall, floorYWall;  // x, y position of the floor texel at the bottom of the wall

        // 4 different wall directions possible
        if (side == 0 && rayDir.x > 0) {
            floorXWall = map_.x;
            floorYWall = map_.y + wallX;
        } else if (side == 0 && rayDir.x < 0) {
            floorXWall = map_.x + 1.0;
            floorYWall = map_.y + wallX;
        } else if (side == 1 && rayDir.y > 0) {
            floorXWall = map_.x + wallX;
            floorYWall = map_.y;
        } else {
            floorXWall = map_.x + wallX;
            floorYWall = map_.y + 1.0;
        }

        float distWall, distPlayer;

        distWall = perpWallDist;
        distPlayer = 0.0;

        if (drawEnd < 0)
            drawEnd = H;  // becomes < 0 when the integer overflows

        // draw the floor from drawEnd to the bottom of the screen
        for (int y = drawEnd + 1; y < H; y++) {
            float currentDist = distTable[y];

            float weight = (currentDist - distPlayer) / (distWall - distPlayer);

            float currentFloorX = weight * floorXWall + (1.0 - weight) * player.position.x;
            float currentFloorY = weight * floorYWall + (1.0 - weight) * player.position.y;

            auto currentTile = map.getTile(currentFloorX, currentFloorY);
            if (!currentTile.floor && !currentTile.ceiling)
                continue;

            int floorTexX, floorTexY;
            floorTexX = int(currentFloorX * 64) % 64;
            floorTexY = int(currentFloorY * 64) % 64;

            // floor
            if (currentTile.floor)
                buffer.append(sf::Vertex(sf::Vector2f(x, y - 1), currentTile.floor->getPixel(floorTexX, floorTexY)));

            // ceil (currently symmetrycal)
            if (currentTile.ceiling)
                buffer.append(sf::Vertex(sf::Vector2f(x, H - y + 1), currentTile.ceiling->getPixel(floorTexX, floorTexY)));
        }
    }

    window.clear(sf::Color(135, 206, 235));

    window.draw(buffer);

    window.display();
}

int main() {
    auto map = Map("../map1.json");

    sf::RenderWindow window(sf::VideoMode(W, H), "BESHRAY", sf::Style::Default);

    auto player = Player();
    auto raycaster = Raycaster();

    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point end;
    float fps;

    window.setFramerateLimit(2000);

    sf::Font font;
    font.loadFromFile("/usr/share/fonts/TTF/DejaVuSerif.ttf");

    while (window.isOpen()) {
        start = std::chrono::high_resolution_clock::now();

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::W)
                    player.moveForward(0.2);
                if (event.key.code == sf::Keyboard::S)
                    player.moveBackwards(0.2);
                if (event.key.code == sf::Keyboard::D)
                    player.rotateRight(0.1);
                if (event.key.code == sf::Keyboard::A)
                    player.rotateLeft(0.1);
            }
        }

        raycaster.renderFrame(window, player, map);

        end = std::chrono::high_resolution_clock::now();
        fps = (float)1e9 / (float)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        window.setTitle(std::to_string(fps));
    }

    return 0;
}