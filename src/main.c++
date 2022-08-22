#include "engine.h++"

// TODO: move DDA to another function +
//       my own vec2 with implicit convertion to sfml vectors +
//       detect side of a wall
//       create function to check ray intersection with entity
//       change formatting to godot-like

class ExampleGame : public beshray::Engine {
  public:
    ExampleGame(const unsigned w = 960, const unsigned h = 540) : beshray::Engine(w, h) {}

    bool onCreate() override
    {
        map = beshray::Map("../map1.json");
        camera = beshray::Camera(5, 12, 120);

        sf::Image b;
        b.loadFromFile("../resources/barrel.png");

        std::shared_ptr<sf::Image> barrel = std::make_shared<sf::Image>(b);

        sprites.push_back(beshray::Sprite{{5, 13}, barrel});
        sprites.push_back(beshray::Sprite{{6, 12}, barrel});
        sprites.push_back(beshray::Sprite{{5, 12.5}, barrel});
        sprites.push_back(beshray::Sprite{{6, 13.5}, barrel});
        sprites.push_back(beshray::Sprite{{8, 12}, barrel});

        return true;
    }

    void onUpdate(float deltaTime) override
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