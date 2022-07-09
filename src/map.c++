#include "map.h++"

void Map::draw(Framebuffer& fb) {
    const size_t rect_w = fb.w / w;
    const size_t rect_h = fb.h / h;

    for (size_t j = 0; j < h; j++) {
        for (size_t i = 0; i < w; i++) {
            if (map[i + j * w] == 0)
                continue;

            size_t rect_x = i * rect_w;
            size_t rect_y = j * rect_h;
            fb.setRectangle(rect_x, rect_y, rect_w, rect_h, RGB{0, 255, 255, 255});
        }
    }
}

bool Map::isEmpty(const size_t x, const size_t y) const {
    return map[x + y * w] == 0;
}