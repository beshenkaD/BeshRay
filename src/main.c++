#include "engine.h++"
#include "map.h++"
#include <cmath>

#include <chrono>
#include <iostream>

#include "math/vector.h++"

// TODO: move DDA to another function
//       my own vec2 with implicit convertion to sfml vectors +
//       detect side of a wall
//       change 'raycaster' class to 'engine' class
//       set camera inside engine class
//       ? thin walls ?
//       ? 45-degree walls ?
//       ? pillar-like walls ?
//       create function to check ray intersection with entity
//       change formatting to godot-like
//       add namespace for engine

int main()
{
    auto engine = beshray::Engine(1280, 720);

    engine.map = beshray::Map("../map1.json");
    engine.camera = beshray::Camera();

    engine.camera.pos.x = 5;
    engine.camera.pos.y = 12;

    engine.setInterlacing(false);

    sf::RenderWindow window(sf::VideoMode(engine.width, engine.height), "BESHRAY", sf::Style::Default);

    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point end;
    float fps = NAN;

    // window.setFramerateLimit(60);

    sf::Font font;
    font.loadFromFile("/usr/share/fonts/TTF/DejaVuSerif.ttf");

    sf::Image b;
    b.loadFromFile("../resources/barrel.png");

    std::shared_ptr<sf::Image> barrel = std::make_shared<sf::Image>(b);

    engine.entities.push_back(beshray::Entity{{5, 13}, 1, 1, barrel});
    engine.entities.push_back(beshray::Entity{{6, 12}, 1, 1, barrel});
    engine.entities.push_back(beshray::Entity{{5, 12.5}, 1, 1, barrel});
    engine.entities.push_back(beshray::Entity{{6, 13.5}, 1, 1, barrel});
    engine.entities.push_back(beshray::Entity{{8, 12}, 1, 1, barrel});

    while (window.isOpen()) {
        start = std::chrono::high_resolution_clock::now();

        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed) {
                //     if (event.key.code == sf::Keyboard::W)
                //         engine.camera.move(0.2);
                //     if (event.key.code == sf::Keyboard::S)
                //         engine.camera.move(-0.13);
                //     if (event.key.code == sf::Keyboard::D)
                //         engine.camera.rotate(-0.05);
                //     if (event.key.code == sf::Keyboard::A)
                //         engine.camera.rotate(0.05);
                if (event.key.code == sf::Keyboard::Up)
                    engine.camera.tilt(5);
                if (event.key.code == sf::Keyboard::Down)
                    engine.camera.tilt(-5);
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            engine.camera.move(0.1);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            engine.camera.move(-0.05);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            engine.camera.rotate(-0.05);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            engine.camera.rotate(0.05);
        }

        window.clear(sf::Color::Black);
        engine.render();
        engine.present(window);

        end = std::chrono::high_resolution_clock::now();
        fps = (float)1e9 / (float)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

        sf::Text t(std::to_string(fps), font);
        t.setFillColor(sf::Color::Yellow);
        t.setOutlineThickness(5);
        t.setOutlineColor(sf::Color::Black);

        window.draw(t);

        window.display();
    }

    return 0;
}