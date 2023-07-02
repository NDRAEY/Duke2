#include <unordered_set>
#include "palette.hpp"
#include <vector>

std::size_t PixelHash::operator()(const Pixel& pixel) const {
    std::size_t seed = 0;
    seed ^= std::hash<char>{}(pixel.r) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= std::hash<char>{}(pixel.g) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= std::hash<char>{}(pixel.b) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= std::hash<bool>{}(pixel.alpha) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    return seed;
}

std::vector<Pixel> buildPalette(const char* pixelData, int width, int height, bool hasAlpha) {
    std::unordered_set<Pixel, PixelHash> uniquePixels;

    for (int i = 0; i < width * height; ++i)
    {
        Pixel pixel;
        pixel.r = pixelData[i * (hasAlpha ? 4 : 3)];
        pixel.g = pixelData[i * (hasAlpha ? 4 : 3) + 1];
        pixel.b = pixelData[i * (hasAlpha ? 4 : 3) + 2];
        
        if(hasAlpha)
        	pixel.alpha = pixelData[i * (hasAlpha ? 4 : 3) + 3];
        else
        	pixel.alpha = 0xFF;

        uniquePixels.insert(pixel);
    }

    return std::vector<Pixel>(uniquePixels.begin(), uniquePixels.end());
}

/*
int main() {
    const char* pixelData = "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x01\x02\x03\x04";
    int width = 2;
    int height = 2;
    bool hasAlpha = true;

    std::vector<Pixel> palette = buildPalette(pixelData, width, height, hasAlpha);

    for (const auto& pixel : palette) {
        std::cout << "R: " << static_cast<int>(pixel.r)
                  << " G: " << static_cast<int>(pixel.g)
                  << " B: " << static_cast<int>(pixel.b)
                  << " Alpha: " << (int)pixel.alpha << std::endl;
    }

    return 0;
}
*/
