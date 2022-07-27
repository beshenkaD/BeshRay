#include "map.h++"
#include <SFML/Graphics.hpp>
#include <chrono>
#include <cmath>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>

const auto W = 960;
const auto H = 540;

struct Player {
    sf::Vector2f position;
    sf::Vector2f direction;

    //    private:
    sf::Vector2f camera; // camera plane (line)

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
    sf::Vector2f rotate(sf::Vector2f &v, float angle);
};

void Player::pitchUp(float amount)
{
    pitch += amount;
}

void Player::pitchDown(float amount)
{
    pitch -= amount;
}

void Player::moveForward(float speed)
{
    position += speed * direction;
}

void Player::moveBackwards(float speed)
{
    position -= speed * direction;
}

void Player::moveUp(float speed)
{
    posZ += speed;
}

void Player::moveDown(float speed)
{
    posZ -= speed;
}

sf::Vector2f Player::rotate(sf::Vector2f &v, float angle)
{
    return sf::Vector2f(
        v.x * std::cos(angle) - v.y * std::sin(angle),
        v.x * std::sin(angle) + v.y * std::cos(angle));
}

void Player::rotateLeft(float speed)
{
    direction = rotate(direction, speed);
    camera = rotate(camera, speed);
}

void Player::rotateRight(float speed)
{
    speed = -speed;

    direction = rotate(direction, speed);
    camera = rotate(camera, speed);
}

class Raycaster
{
private:
    // move it to another class
    sf::Uint8 *pixelBuffer;
    sf::Texture pixelBufferTex;

    void plotPixel(int x, int y, sf::Color c);

public:
    Raycaster()
    {
        pixelBuffer = new sf::Uint8[W * H * 4];
        pixelBufferTex.create(W, H);
    };

    ~Raycaster()
    {
        delete[] pixelBuffer;
    }

    void renderFrame(sf::RenderWindow &window, Player &player, Map &map);
    void present(sf::RenderWindow &window);
};

void Raycaster::present(sf::RenderWindow &window)
{
    pixelBufferTex.update(pixelBuffer);
    sf::Sprite s{pixelBufferTex};
    window.draw(s);
}

void Raycaster::plotPixel(int x, int y, sf::Color c)
{
    unsigned int index = (y * (W * 4)) + (x * 4);
    pixelBuffer[index] = c.r;
    pixelBuffer[index + 1] = c.g;
    pixelBuffer[index + 2] = c.b;
    pixelBuffer[index + 3] = c.a;
}

void Raycaster::renderFrame(sf::RenderWindow &window, Player &player, Map &map)
{
    memset(pixelBuffer, 0, W * H * 4 * sizeof(sf::Uint8));

    for (int x = 0; x < W; x++) {
        // x-coordinate in camera plane(line). 1 - top right, 0 - center,
        // -1 - top left
        float camX = 2 * x / float(W) - 1;

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

        bool hit = false;  // was there a wall hit?
        bool side = false; // was a NS or a EW wall hit?

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

        // Calculate distance projected on camera direction (Euclidean
        // distance would give fisheye effect!)
        float perpWallDist;

        // perform DDA
        while (!hit) {
            // jump to next map square, either in x-direction, or in
            // y-direction
            if (sideDist.x < sideDist.y) {
                sideDist.x += deltaDist.x;
                map_.x += step.x;
                side = false;
                perpWallDist = (sideDist.x - deltaDist.x);
            } else {
                sideDist.y += deltaDist.y;
                map_.y += step.y;
                side = true;
                perpWallDist = (sideDist.y - deltaDist.y);
            }

            // Check if ray has hit a wall
            if (map.getTile(map_.x, map_.y).isWall)
                hit = true;
        }

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
        float wallX; // where the wall was hit
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
        float texPos =
            (drawStart - player.pitch - (player.posZ / perpWallDist) - (H >> 1) + (lineHeight >> 1)) * texStep;

        // NEW 1 loop
        for (int y = 0; y < H; y++) {
            if (y <= drawStart - 1) { // ceiling
                float currentDist = (H - (2.0 * player.posZ)) / (H - 2.0 * (y - player.pitch));

                sf::Vector2f planePoint = player.position + rayDir * (currentDist);
                int planeTileX = int(planePoint.x);
                int planeTileY = int(planePoint.y);

                auto currentTile = map.getTile(planeTileX, planeTileY);
                if (!currentTile.floor && !currentTile.ceiling)
                    continue;

                float planeSampleX = planePoint.x - planeTileX;
                float planeSampleY = planePoint.y - planeTileY;

                int floorTexX = int(planeSampleX * 64) & 63;
                int floorTexY = int(planeSampleY * 64) & 63;

                if (currentTile.ceiling) {
                    auto c = currentTile.ceiling->getPixel(floorTexX, floorTexY);
                    plotPixel(x, y, c);
                }
            } else if (y >= drawStart && y <= drawEnd) { // walls
                int texY = (int)texPos & (wallTexture->getSize().y - 1);
                texPos += texStep;

                auto c = wallTexture->getPixel(texX, texY);
                plotPixel(x, y, c);
            } else { // floor
                float currentDist = (H + (2.0 * player.posZ)) / (2.0 * (y - player.pitch) - H);

                // TEST
                sf::Vector2f planePoint = player.position + rayDir * (currentDist);
                int planeTileX = int(planePoint.x);
                int planeTileY = int(planePoint.y);

                auto currentTile = map.getTile(planeTileX, planeTileY);
                if (!currentTile.floor && !currentTile.ceiling)
                    continue;

                float planeSampleX = planePoint.x - planeTileX;
                float planeSampleY = planePoint.y - planeTileY;

                int floorTexX = int(planeSampleX * 64) & 63;
                int floorTexY = int(planeSampleY * 64) & 63;
                // TEST END

                // floor
                if (currentTile.floor) {
                    // auto c =
                    // currentTile.floor->getPixel(floorTexX,
                    // floorTexY);
                    auto c = currentTile.floor->getPixel(floorTexX, floorTexY);

                    plotPixel(x, y, c);
                }
            }
        }
    }
}

int main()
{
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
                    player.pitchUp(20);
                if (event.key.code == sf::Keyboard::Down)
                    player.pitchDown(20);
                if (event.key.code == sf::Keyboard::Space)
                    player.moveUp(20);
                if (event.key.code == sf::Keyboard::Num0)
                    player.moveDown(20);
            }
        }

        window.clear(sf::Color(135, 206, 235));
        raycaster.renderFrame(window, player, map);
        raycaster.present(window);

        end = std::chrono::high_resolution_clock::now();
        fps = (float)1e9 / (float)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        sf::Text t(std::to_string(fps), font);

        window.draw(t);

        window.display();
    }

    return 0;
}