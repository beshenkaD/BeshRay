#include <math.h>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Transform.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <chrono>
#include <cmath>
#include <string>

const size_t map_w = 16;
const size_t map_h = 16;

// clang-format off
const int map[map_w*map_h] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 4, 0, 0, 4, 2, 2, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 4, 0, 0, 2, 0, 2, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 4, 0, 0, 2, 2, 2, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};
// clang-format on

const auto W = 640;
const auto H = 480;

struct Player {
    sf::Vector2f position;
    sf::Vector2f direction;

    //    private:
    sf::Vector2f camera;  // camera plane (line)

    Player() : position(sf::Vector2f(22, 12)), direction(sf::Vector2f(-1, 0)), camera(sf::Vector2f(0, 1)){};

    void moveForward(float speed);
    void moveBackwards(float speed);
    void rotateLeft(float speed);
    void rotateRight(float speed);
};

void Player::moveForward(float speed) {
    position += speed * direction;
}

void Player::moveBackwards(float speed) {
    position -= speed * direction;
}

void Player::rotateLeft(float speed) {
    auto oldDirectionX = direction.x;

    direction.x = direction.x * cos(speed) - direction.y * sin(speed);
    direction.y = oldDirectionX * sin(speed) + direction.y * cos(speed);

    auto oldCameraX = camera.x;
    camera.x = camera.x * cos(speed) - camera.y * sin(speed);
    camera.y = oldCameraX * sin(speed) + camera.y * cos(speed);
}

void Player::rotateRight(float speed) {
    speed = -speed;

    auto oldDirectionX = direction.x;

    direction.x = direction.x * cos(speed) - direction.y * sin(speed);
    direction.y = oldDirectionX * sin(speed) + direction.y * cos(speed);

    auto oldCameraX = camera.x;
    camera.x = camera.x * cos(speed) - camera.y * sin(speed);
    camera.y = oldCameraX * sin(speed) + camera.y * cos(speed);
}

class Raycaster {
    void renderFrame();
};

void Raycaster::renderFrame() {}

int main() {
    sf::RenderWindow window(sf::VideoMode(W, H), "BESHRAY");

    auto player = Player();

    // this are VECTORS
    float posX = 22, posY = 12;    // player start position
    float dirX = -1, dirY = 0;     // direction vector for player
    float planeX = 0, planeY = 1;  // camera plane

    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point end;
    float fps;

    while (window.isOpen()) {
        start = std::chrono::high_resolution_clock::now();

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            const auto speed = 0.4;
            const auto rotSpeed = 0.2;

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::W)
                    player.moveForward(speed);
                if (event.key.code == sf::Keyboard::S)
                    player.moveBackwards(speed);
                if (event.key.code == sf::Keyboard::D)
                    player.rotateRight(rotSpeed);
                if (event.key.code == sf::Keyboard::A)
                    player.rotateLeft(rotSpeed);
            }
        }

        window.clear(sf::Color::Black);

        for (int x = 0; x < W; x++) {
            // ray position and direction
            float camX = 2 * x / float(W) - 1;  // x-coordinate in camera plane(line). 1 - top right, 0 - center, -1 - top left
            // v = planeDir, d = dir, p = plane, c = camX (scalar)
            //             _   _    _
            // this means: v = d + cp
            float rayDirX = dirX + planeX * camX;
            float rayDirY = dirY + planeY * camX;

            // which box of the map we're in
            int mapX = static_cast<int>(posX);
            int mapY = static_cast<int>(posY);

            // length of ray from current position to next x or y-side
            float sideDistX;
            float sideDistY;

            // length of ray from one x or y-side to next x or y-side
            float deltaDistX = /*(rayDirX == 0) ? 1e30 :*/ std::abs(1 / rayDirX);
            float deltaDistY = /*(rayDirY == 0) ? 1e30 :*/ std::abs(1 / rayDirY);
            float perpWallDist;

            // what direction to step in x or y-direction (either +1 or -1)
            int stepX;
            int stepY;

            int hit = 0;  // was there a wall hit?
            int side;     // was a NS or a EW wall hit?

            // calculate step and initial sideDist
            if (rayDirX < 0) {
                stepX = -1;
                sideDistX = (posX - mapX) * deltaDistX;
            } else {
                stepX = 1;
                sideDistX = (mapX + 1.0 - posX) * deltaDistX;
            }
            if (rayDirY < 0) {
                stepY = -1;
                sideDistY = (posY - mapY) * deltaDistY;
            } else {
                stepY = 1;
                sideDistY = (mapY + 1.0 - posY) * deltaDistY;
            }

            // perform DDA
            while (hit == 0) {
                // jump to next map square, either in x-direction, or in y-direction
                if (sideDistX < sideDistY) {
                    sideDistX += deltaDistX;
                    mapX += stepX;
                    side = 0;
                } else {
                    sideDistY += deltaDistY;
                    mapY += stepY;
                    side = 1;
                }

                // Check if ray has hit a wall
                if (map[mapX + mapY * map_w] > 0)
                    hit = 1;
            }

            // Calculate distance projected on camera direction (Euclidean distance would give fisheye effect!)
            if (side == 0)
                perpWallDist = (sideDistX - deltaDistX);
            else
                perpWallDist = (sideDistY - deltaDistY);

            // Calculate height of line to draw on screen
            int lineHeight = (int)(H / perpWallDist);

            // calculate lowest and highest pixel to fill in current stripe
            int drawEnd = lineHeight / 2 + H / 2;
            if (drawEnd >= H)
                drawEnd = H - 1;

            int drawStart = H / 2 - lineHeight / 2;
            if (drawStart < 0)
                drawStart = 0;

            sf::Color color;
            switch (map[mapX + mapY * map_w]) {
                case 1:
                    color = sf::Color::Red;
                    break;
                case 2:
                    color = sf::Color::Cyan;
                    break;
                case 3:
                    color = sf::Color::White;
                    break;
                case 4:
                    color = sf::Color::Yellow;
                    break;
                default:
                    color = sf::Color::Magenta;
                    break;
            }

            if (side == 1) {
                color.r /= 2;
                color.g /= 2;
                color.b /= 2;
            }

            // draw vertical line
            sf::Vertex line[2] = {
                sf::Vertex(sf::Vector2f(x, drawStart), color),
                sf::Vertex(sf::Vector2f(x, drawEnd), color),
            };

            window.draw(line, 2, sf::Lines);
        }

        window.display();

        end = std::chrono::high_resolution_clock::now();
        fps = (float)1e9 / (float)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        window.setTitle(std::to_string(fps));
    }

    return 0;
}