#include <SFML/Config.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <cstddef>
#include <numbers>
#include "framebuffer.h++"
#include "map.h++"
#include "player.h++"

const size_t map_w = 16;
const size_t map_h = 16;

// clang-format off
const int map[map_w*map_h] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};
// clang-format on

int main() {
    auto fb = Framebuffer(640, 320);

    float player_x = 3.456;
    float player_y = 2.345;
    float player_a = 1.523;
    float deltaX = cos(player_a);
    float deltaY = sin(player_a);

    const float fov = 90 * (std::numbers::pi / 180);

    sf::RenderWindow window(sf::VideoMode(fb.w, fb.h), "BESHRAY");
    window.setFramerateLimit(30);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::W) {
                    player_x += deltaX;
                    player_y += deltaY;
                }
                if (event.key.code == sf::Keyboard::A) {
                    player_a -= 0.1;
                    if (player_a < 0) {
                        player_a += 2 * 3.14;
                    }
                    deltaX = cos(player_a);
                    deltaY = sin(player_a);
                }
                if (event.key.code == sf::Keyboard::S) {
                    player_x -= deltaX;
                    player_y -= deltaY;
                }
                if (event.key.code == sf::Keyboard::D) {
                    player_a += 0.1;
                    if (player_a < 0) {
                        player_a += 2 * 3.14;
                    }
                    deltaX = cos(player_a);
                    deltaY = sin(player_a);
                }
            }
        }

        {
            fb.clear();

            const size_t rect_w = fb.w / (map_w * 2);
            const size_t rect_h = fb.h / map_h;

            for (size_t j = 0; j < map_h; j++) {
                for (size_t i = 0; i < map_w; i++) {
                    if (map[i + j * map_w] == 0)
                        continue;

                    size_t rect_x = i * rect_w;
                    size_t rect_y = j * rect_h;
                    fb.setRectangle(rect_x, rect_y, rect_w, rect_h, RGB{0, 255, 255, 255});
                }
            }

            fb.setRectangle(player_x * rect_w, player_y * rect_h, 5, 5, RGB{255, 255, 255, 255});

            for (size_t i = 0; i < fb.w / 2; i++) {
                float angle = player_a - fov / 2 + fov * i / (float(fb.w) / 2);

                for (float t = 0; t < 20; t += .05) {
                    float cx = player_x + t * cos(angle);
                    float cy = player_y + t * sin(angle);

                    size_t pix_x = cx * rect_w;
                    size_t pix_y = cy * rect_h;
                    fb.setPixel(pix_x, pix_y, RGB{160, 160, 160, 255});

                    if (map[int(cx) + int(cy) * map_w] != 0) {
                        size_t column_height = fb.h / (t * cos(angle - player_a));
                        fb.setRectangle(fb.w / 2 + i, fb.h / 2 - column_height / 2, 1, column_height, RGB{0, 255, 255, 255});
                        break;
                    }
                }
            }
        }

        {
            size_t i = 0;
            sf::Uint8* pixels = new sf::Uint8[fb.h * fb.w * 4];
            for (auto col : fb.image) {
                pixels[i++] = col.r;
                pixels[i++] = col.g;
                pixels[i++] = col.b;
                pixels[i++] = col.a;
            }

            window.clear(sf::Color::Black);

            sf::Image img;
            img.create(fb.w, fb.h, pixels);

            sf::Texture tex;
            tex.setSmooth(false);
            tex.setRepeated(false);
            tex.loadFromImage(img);

            sf::Sprite spr;
            spr.setTexture(tex);

            window.draw(spr);

            window.display();
        }
    }

    return 0;
}