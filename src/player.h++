#pragma once
#include <cmath>
#include <cstddef>
#include "framebuffer.h++"

struct Player {
    enum Direction {
        UP,
        DOWN,
        LEFT,
        RIGHT,
    };

    float x, y;
    float angle;  // angle between x axis and player's direction
    float deltaX;
    float deltaY;

    Player() : x(256), y(256), angle(1), deltaX(cos(angle) * 5), deltaY(sin(angle) * 5){};

    void draw(Framebuffer& fb);
    void move(Direction d);
};