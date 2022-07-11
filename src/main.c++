#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Color.hpp>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <numbers>
#include <string>
#include <unordered_map>
#include <vector>

const size_t map_w = 16;
const size_t map_h = 16;

// clang-format off
const int map[map_w*map_h] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 4, 0, 0, 4, 2, 2, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 4, 0, 0, 2, 0, 2, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 4, 0, 0, 2, 2, 2, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 1,
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

struct Texture {
    size_t w, h;
    sf::Texture tex;

    Texture(const std::string& filename);
};

// Texture::Texture(const std::string& filename) {

// }

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

    float height = 0.666;

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
    sf::VertexArray floorPoints;
    sf::Image floorTex;

    std::vector<float> distTable;

    void calculateDistTable();

   public:
    Raycaster() {
        if (!texture.loadFromFile("../walls.png")) {
            abort();
        }

        if (!floorTex.loadFromFile("../pics/mossy.png")) {
            abort();
        }
        texture.setSmooth(false);

        state = sf::RenderStates(&texture);
        lines = sf::VertexArray(sf::Lines, W);
        floorPoints = sf::VertexArray(sf::PrimitiveType::Points, W * H);

        calculateDistTable();
    };

    void renderFrame(sf::RenderWindow& window, Player& player);
};

void Raycaster::calculateDistTable() {
    for (int y = 0; y < H; y++) {
        distTable.push_back(H / (2.0 * y - H));
    }
}

void Raycaster::renderFrame(sf::RenderWindow& window, Player& player) {
    lines.resize(0);
    floorPoints.resize(0);

    // Render floor
    // for (int y = 0; y < H; y++) {
    //     auto rayDir0 = sf::Vector2f(player.direction.x - player.camera.x, player.direction.y - player.camera.y);
    //     auto rayDir1 = sf::Vector2f(player.direction.x + player.camera.x, player.direction.y + player.camera.y);

    //     int p = y - H / 2;      // horizon
    //     float posZ = 0.5f * H;  // position of the camera

    //     // Horizontal distance from the camera to the floor for the current row.
    //     // 0.5 is the z position exactly in the middle between floor and ceiling.
    //     float rowDistance = posZ / p;

    //     auto floorStep = sf::Vector2f(rowDistance * (rayDir1.x - rayDir0.x) / W, rowDistance * (rayDir1.y - rayDir0.y) / W);
    //     auto floor = sf::Vector2f(player.position.x + rowDistance * rayDir0.x, player.position.y + rowDistance * rayDir0.y);

    //     for (int x = 0; x < W; x++) {
    //         int tx = (int)(64 * (floor.x - int(floor.x))) & (64 - 1);
    //         int ty = (int)(64 * (floor.y - int(floor.y))) & (64 - 1);

    //         floor += floorStep;

    //         // floor
    //         floorPoints.append(sf::Vertex(sf::Vector2f(x, y), floorTex.getPixel(tx, ty)));
    //         floorPoints.append(sf::Vertex(sf::Vector2f(x, H - y - 1), floorTex.getPixel(tx, ty)));
    //     }
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
            if (map[map_.x + map_.y * map_w] > 0)
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

        int texNum = static_cast<int>(textureTypes.find(map[map_.x + map_.y * map_w])->second);
        sf::Vector2i texCoords(texNum * TEX_WALL % TEX_SIZE, texNum * TEX_WALL / TEX_SIZE * TEX_WALL);

        // x coordinate of the texture
        int texX = static_cast<int>(wallX * TEX_WALL);
        if ((!side && rayDir.x <= 0) || (side && rayDir.y >= 0)) {
            texX = TEX_WALL - texX - 1;
        }

        texCoords.x += texX;

        // shading
        auto color_ = sf::Color::White;
        if (side) {
            color_.r >>= 1;
            color_.g >>= 1;
            color_.b >>= 1;
        }

        lines.append(sf::Vertex(sf::Vector2f(x, drawStart), color_, sf::Vector2f(texCoords.x, texCoords.y)));
        lines.append(sf::Vertex(sf::Vector2f(x, drawEnd), color_, sf::Vector2f(texCoords.x, texCoords.y + TEX_WALL)));

        // FLOOR
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

            int floorTexX, floorTexY;
            floorTexX = int(currentFloorX * 64) % 64;
            floorTexY = int(currentFloorY * 64) % 64;

            // floor
            floorPoints.append(sf::Vertex(sf::Vector2f(x, y), floorTex.getPixel(floorTexX, floorTexY)));
            // ceil
            if (map[int(currentFloorX) + int(currentFloorY) * map_w] != -1)
                floorPoints.append(sf::Vertex(sf::Vector2f(x, H - y), floorTex.getPixel(floorTexX, floorTexY)));
        }
    }

    window.clear();

    window.draw(floorPoints);
    window.draw(lines, state);

    window.display();
}

int main() {
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

        raycaster.renderFrame(window, player);

        end = std::chrono::high_resolution_clock::now();
        fps = (float)1e9 / (float)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        window.setTitle(std::to_string(fps));
    }

    return 0;
}