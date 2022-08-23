#include "engine.h++"
#include <chrono>
#include <cmath>
#include <iostream>

// TODO: move DDA to another function +
//       my own vec2 with implicit convertion to sfml vectors +
//       detect side of a wall
//       create function to check ray intersection with entity
//       change formatting to godot-like

class ExampleGame : public beshray::Engine {
  public:
    ExampleGame(const unsigned w = 960, const unsigned h = 540) : beshray::Engine(w, h) {}
    ~ExampleGame() override = default;

    bool onCreate() override
    {
        map = beshray::Map("../map1.json");
        camera = beshray::Camera(5, 12, 120, 0, 0);

        sf::Image b;
        b.loadFromFile("../resources/barrel.png");

        auto barrel = std::make_shared<sf::Image>(b);

        sprites.push_back(beshray::Sprite{{5, 13}, barrel});
        sprites.push_back(beshray::Sprite{{6, 12}, barrel});
        sprites.push_back(beshray::Sprite{{5, 12.5}, barrel});
        sprites.push_back(beshray::Sprite{{6, 13.5}, barrel});
        sprites.push_back(beshray::Sprite{{8, 12}, barrel});

        setInterlacing(true);
        return true;
    }

    void onUpdate(const float deltaTime) override
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            camera.move(5 * deltaTime);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            camera.move(-3 * deltaTime);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            camera.rotate(-3 * deltaTime);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            camera.rotate(3 * deltaTime);
        }

        render();
    }
};

int main()
{
    auto game = ExampleGame(1280, 720);
    return game.start("example game");
}