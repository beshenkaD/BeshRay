#include <SFML/Graphics.hpp>
#include <chrono>
#include <cmath>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include "map.h++"

const auto W = 960;
const auto H = 540;

struct Player {
    sf::Vector2f position;
    sf::Vector2f direction;

    //    private:
    sf::Vector2f camera;  // camera plane (line)

    // TEST
    float pitch = 0;
    float posZ = 0;
    // TEST END

    Player() : position(sf::Vector2f(22, 12)), direction(sf::Vector2f(-1, 0)), camera(sf::Vector2f(0, 1)){};

    void moveForward(float speed);
    void moveBackwards(float speed);
    void rotateLeft(float speed);
    void rotateRight(float speed);
    void pitchUp(float amount);
    void pitchDown(float amount);
    void moveUp(float speed);
    void moveDown(float speed);

   private:
    sf::Vector2f rotate(sf::Vector2f& v, float angle);
};

void Player::pitchUp(float amount) {
    pitch += amount;
}

void Player::pitchDown(float amount) {
    pitch -= amount;
}

void Player::moveForward(float speed) {
    position += speed * direction;
}

void Player::moveBackwards(float speed) {
    position -= speed * direction;
}

void Player::moveUp(float speed) {
    posZ += speed;
}

void Player::moveDown(float speed) {
    posZ -= speed;
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
   private:
    // move it to another class
    sf::Uint8* pixelBuffer;
    sf::Texture pixelBufferTex;

    void plotPixel(int x, int y, sf::Color c);

   public:
    Raycaster() {
        pixelBuffer = new sf::Uint8[W * H * 4];
        pixelBufferTex.create(W, H);
    };

    ~Raycaster() { delete[] pixelBuffer; }

    void renderFrame(sf::RenderWindow& window, Player& player, Map& map);
};

void Raycaster::plotPixel(int x, int y, sf::Color c) {
    unsigned int index = (y * (W * 4)) + (x * 4);
    pixelBuffer[index] = c.r;
    pixelBuffer[index + 1] = c.g;
    pixelBuffer[index + 2] = c.b;
    pixelBuffer[index + 3] = c.a;
}

void Raycaster::renderFrame(sf::RenderWindow& window, Player& player, Map& map) {
    // buffer.resize(0);
    memset(pixelBuffer, 0, W * H * 4 * sizeof(sf::Uint8));

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
        int drawStart = -(lineHeight >> 1) + (H >> 1) + player.pitch + (player.posZ / perpWallDist);
        if (drawStart < 0)
            drawStart = 0;

        int drawEnd = (lineHeight >> 1) + (H >> 1) + player.pitch + (player.posZ / perpWallDist);
        if (drawEnd >= H)
            drawEnd = H - 1;

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
        // float texPos = (drawStart - H / 2 + lineHeight / 2) * texStep;
        float texPos = (drawStart - player.pitch - (player.posZ / perpWallDist) - (H >> 1) + (lineHeight >> 1)) * texStep;

        for (int y = drawStart; y < drawEnd + 1; y++) {
            int texY = (int)texPos & (wallTexture->getSize().y - 1);
            texPos += texStep;

            // buffer.append(sf::Vertex(sf::Vector2f(x, y), wallTexture->getPixel(texX, texY) * color_));
            auto c = wallTexture->getPixel(texX, texY);

            plotPixel(x, y, c);
        }

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
            // float currentDist = distTable[y];
            // float currentDist = (H - (2.0 * player.posZ)) / (H - 2.0 * (y - player.pitch)); // ceiling
            float currentDist = (H + (2.0 * player.posZ)) / (2.0 * (y - player.pitch) - H);

            float weight = (currentDist - distPlayer) / (distWall - distPlayer);

            float currentFloorX = weight * floorXWall + (1.0 - weight) * player.position.x;
            float currentFloorY = weight * floorYWall + (1.0 - weight) * player.position.y;

            auto currentTile = map.getTile(currentFloorX, currentFloorY);
            if (!currentTile.floor && !currentTile.ceiling)
                continue;

            int floorTexX, floorTexY;
            // TODO: get texture size
            floorTexX = int(currentFloorX * 64) & 63;
            floorTexY = int(currentFloorY * 64) & 63;

            // floor
            if (currentTile.floor) {
                auto c = currentTile.floor->getPixel(floorTexX, floorTexY);
                plotPixel(x, y, c);
            }
        }

        // Draw ceiling
        for (int y = 0; y < drawStart; y++) {
            // float currentDist = distTable[y];
            float currentDist = (H - (2.0 * player.posZ)) / (H - 2.0 * (y - player.pitch));

            float weight = (currentDist - distPlayer) / (distWall - distPlayer);

            float currentFloorX = weight * floorXWall + (1.0 - weight) * player.position.x;
            float currentFloorY = weight * floorYWall + (1.0 - weight) * player.position.y;

            auto currentTile = map.getTile(currentFloorX, currentFloorY);
            if (!currentTile.floor && !currentTile.ceiling)
                continue;

            int floorTexX, floorTexY;
            floorTexX = int(currentFloorX * 64) & 63;
            floorTexY = int(currentFloorY * 64) & 63;

            if (currentTile.ceiling) {
                auto c = currentTile.ceiling->getPixel(floorTexX, floorTexY);
                plotPixel(x, y, c);
            }
        }
    }

    pixelBufferTex.update(pixelBuffer);

    sf::Sprite s(pixelBufferTex);

    window.draw(s);

    // window.draw(buffer);
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
                if (event.key.code == sf::Keyboard::Up)
                    player.pitchUp(5);
                if (event.key.code == sf::Keyboard::Down)
                    player.pitchDown(5);
                if (event.key.code == sf::Keyboard::Space)
                    player.moveUp(1);
                if (event.key.code == sf::Keyboard::Num0)
                    player.moveDown(1);
            }
        }

        window.clear(sf::Color(135, 206, 235));
        raycaster.renderFrame(window, player, map);

        end = std::chrono::high_resolution_clock::now();
        fps = (float)1e9 / (float)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        sf::Text t(std::to_string(fps), font);

        window.draw(t);

        window.display();
    }

    return 0;
}