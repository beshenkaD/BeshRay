#include "framebuffer.h++"
#include <cassert>
#include <fstream>
#include <vector>

void Framebuffer::renderToPPM(const std::string filename) {
    std::ofstream out(filename, std::ios::binary);

    out << "P6\n" << w << ' ' << h << "\n255\n";

    for (size_t i = 0; i < h * w; i++) {
        auto col = image[i];

        out << static_cast<char>(col.r) << static_cast<char>(col.g) << static_cast<char>(col.b);
    }

    out.close();
}

void Framebuffer::setPixel(const size_t x, const size_t y, RGB color) {
    assert(image.size() == w * h && x < w && y < h);

    image[x + y * w] = color;
}

void Framebuffer::setRectangle(const size_t rx, const size_t ry, const size_t rw, const size_t rh, const RGB color) {
    assert(image.size() == w * h);

    for (size_t i = 0; i < rw; i++) {
        for (size_t j = 0; j < rh; j++) {
            size_t cx = rx + i;
            size_t cy = ry + j;

            if (cx < w && cy < h)
                setPixel(cx, cy, color);
        }
    }
}

void Framebuffer::clear() {
    image = std::vector<RGB>(h * w, RGB{255, 255, 255, 255});
}