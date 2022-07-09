#pragma once

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

struct RGB {
    uint8_t r, g, b, a;
};

struct Framebuffer {
    size_t w, h;
    std::vector<RGB> image = std::vector<RGB>(w * h, RGB{255, 255, 255, 255});

    constexpr Framebuffer(const size_t W, const size_t H) : w(W), h(H){};

    void renderToPPM(const std::string filename);
    void setPixel(const size_t x, const size_t y, const RGB color);
    void setRectangle(const size_t x, const size_t y, const size_t w, const size_t h, const RGB color);
    void clear();
};
