#pragma once

#include <iostream>

struct Pixel {
    char r, g, b;
    char alpha;

    bool operator==(const Pixel& other) const {
        return r == other.r && g == other.g && b == other.b && alpha == other.alpha;
    }
};

struct PixelHash {
	std::size_t operator()(const Pixel& pixel) const;
};

std::vector<Pixel> buildPalette(const char* pixelData, int width, int height, bool hasAlpha);
