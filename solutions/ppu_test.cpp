#include "headers/bus.h"
#include "headers/ppu.h"
#include <fstream>
#include <iostream>

int main() {
    Bus bus;
    PPU ppu(bus);

    // Generate a simple CHR: 1 pattern table (4096 bytes)
    bus.chrRom.resize(4096);
    for (int tile = 0; tile < 256; ++tile) {
        int base = tile * 16;
        for (int row = 0; row < 8; ++row) {
            // simple vertical stripes per tile: alternate columns
            uint8_t pattern = (row % 2) ? 0xAA : 0x55; // 10101010 / 01010101
            bus.chrRom[base + row] = pattern; // low plane
            bus.chrRom[base + row + 8] = (tile & 1) ? 0xFF : 0x00; // high plane
        }
    }

    std::vector<uint32_t> pixels;
    int w=0,h=0;
    if (!ppu.RenderPatternTable(0, pixels, w, h)) {
        std::cerr << "Failed to render pattern table\n";
        return 1;
    }

    std::ofstream out("ppu_out.ppm", std::ios::binary);
    out << "P6\n" << w << " " << h << "\n255\n";
    for (int i = 0; i < w*h; ++i) {
        uint32_t c = pixels[i];
        unsigned char r = (c >> 16) & 0xFF;
        unsigned char g = (c >> 8) & 0xFF;
        unsigned char b = (c >> 0) & 0xFF;
        out.put(r); out.put(g); out.put(b);
    }
    out.close();
    std::cout << "Wrote ppu_out.ppm (" << w << "x" << h << ")\n";
    return 0;
}
