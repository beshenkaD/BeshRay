#include "player.h++"
#include <cmath>
#include <cstddef>

void Player::draw(Framebuffer& fb) {
    fb.setRectangle(x, y, 8, 8, RGB{255, 0, 0, 255});

    // for (float t = 0; t < 50; t += .05) {
    //     float cx = x + t * cos(angle);
    //     float cy = y + t * sin(angle);

    //     fb.setPixel(cx, cy, RGB{255, 255, 0, 255});
    // }
}

void Player::move(Player::Direction d) {
    switch (d) {
        case UP:
            x += deltaX;
            y += deltaY;
            break;
        case DOWN:
            x -= deltaX;
            y -= deltaY;
            break;
        case LEFT:
            angle -= 0.1;
            if (angle < 0) {
                angle += 2 * 3.14;
            }
            deltaX = cos(angle) * 5;
            deltaY = sin(angle) * 5;
            break;
        case RIGHT:
            angle += 0.1;
            if (angle > 2 * 3.14) {
                angle -= 2 * 3.14;
            }
            deltaX = cos(angle) * 5;
            deltaY = sin(angle) * 5;
            break;
    }
}