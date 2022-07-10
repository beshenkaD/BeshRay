#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <string>
#include <unordered_map>

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

const auto W = 1280;
const auto H = 720;

const auto TEX_SIZE = 512;
const auto TEX_WALL = 128;

enum class WallTexture {
    Smiley,
    Red,
    Bush,
    Sky,
    Pink,
    Wallpaper,
    Dirt,
    Exit,
};

const std::unordered_map<int, WallTexture> textureTypes{
    {0, WallTexture::Pink}, {1, WallTexture::Dirt}, {2, WallTexture::Wallpaper}, {3, WallTexture::Bush}, {4, WallTexture::Sky}, {5, WallTexture::Red}, {6, WallTexture::Smiley}, {7, WallTexture::Exit},
};

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
    sf::Texture texture;
    sf::RenderStates state;
    sf::VertexArray lines;

   public:
    Raycaster() {
        if (!texture.loadFromFile("../walls.png")) {
            abort();
        }

        state = sf::RenderStates(&texture);
        lines = sf::VertexArray(sf::Lines, W);
    };

    void renderFrame(sf::RenderWindow& window, Player& player);
};

void Raycaster::renderFrame(sf::RenderWindow& window, Player& player) {
    lines.resize(0);

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

        float perpWallDist;

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
            if (map[map_.x + map_.y * map_w] > 0)
                hit = true;
        }

        // Calculate distance projected on camera direction (Euclidean distance would give fisheye effect!)
        if (side == 0)
            perpWallDist = (sideDist.x - deltaDist.x);
        else
            perpWallDist = (sideDist.y - deltaDist.y);

        // euclidian
        // float e = perpWallDist * std::sqrt(std::pow(rayDir.x, 2) + std::pow(rayDir.y, 2));

        // Calculate height of line to draw on screen
        int lineHeight = (int)(H / perpWallDist);

        // calculate lowest and highest pixel to fill in current stripe
        int drawEnd = lineHeight / 2 + H / 2;
        if (drawEnd >= H)
            drawEnd = H - 1;

        int drawStart = H / 2 - lineHeight / 2;
        if (drawStart < 0)
            drawStart = 0;

        // sf::Color color;
        // switch (map[map_.x + map_.y * map_w]) {
        //     case 1:
        //         color = sf::Color::Red;
        //         break;
        //     case 2:
        //         color = sf::Color::Cyan;
        //         break;
        //     case 3:
        //         color = sf::Color::White;
        //         break;
        //     case 4:
        //         color = sf::Color::Yellow;
        //         break;
        //     default:
        //         color = sf::Color::Magenta;
        //         break;
        // }

        // if (side == 1) {
        //     color.r /= 2;
        //     color.g /= 2;
        //     color.b /= 2;
        // }

        int texNum = static_cast<int>(textureTypes.find(map[map_.x + map_.y * map_w])->second);
        sf::Vector2i texCoords(texNum * TEX_WALL % TEX_SIZE, texNum * TEX_WALL / TEX_SIZE * TEX_WALL);

        float wallX;
        if (!side) {
            wallX = player.position.y + perpWallDist * rayDir.y;
        } else {
            wallX = player.position.x + perpWallDist * rayDir.x;
        }

        wallX -= std::floor(wallX);

        int tex_x = static_cast<int>(wallX * float(TEX_WALL));

        if ((!side && rayDir.x <= 0) || (side && rayDir.y >= 0)) {
            tex_x = TEX_WALL - tex_x - 1;
        }

        texCoords.x += tex_x;

        auto color = sf::Color::White;
        if (side) {
            color.r /= 2;
            color.g /= 2;
            color.b /= 2;
        }

        // lines.append(sf::Vertex(sf::Vector2f(x, drawStart), color));
        // lines.append(sf::Vertex(sf::Vector2f(x, drawEnd), color));

        lines.append(sf::Vertex(sf::Vector2f(x, drawStart), color, sf::Vector2f(texCoords.x, texCoords.y)));
        lines.append(sf::Vertex(sf::Vector2f(x, drawEnd), color, sf::Vector2f(texCoords.x, texCoords.y + TEX_WALL)));

        // window.draw(line, 2, sf::Lines);
    }

    window.clear();
    window.draw(lines, state);
    window.display();
}

int main() {
    sf::RenderWindow window(sf::VideoMode(W, H), "BESHRAY");

    auto player = Player();
    auto raycaster = Raycaster();

    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point end;
    float fps;

    window.setFramerateLimit(1000);

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

        raycaster.renderFrame(window, player);

        end = std::chrono::high_resolution_clock::now();
        fps = (float)1e9 / (float)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

        window.setTitle(std::to_string(fps));
    }

    return 0;
}