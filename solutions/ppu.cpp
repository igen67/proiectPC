#include "headers/ppu.h"
#include "headers/bus.h"
#include "headers/cpu.h"
#include "headers/mapper.h"
#include <cstring>
#include <algorithm>

// Basic NES palette for visualization (64 colors approximated)
const unsigned char NES_COLORS[64] = {
    0x35,0x23,0x16,0x22,0x1C,0x09,0x1D,0x15,0x20,0x00,0x27,0x05,0x04,0x28,0x08,0x20,
    0x21,0x3E,0x1F,0x29,0x3C,0x32,0x36,0x12,0x3F,0x2B,0x2E,0x1E,0x3D,0x2D,0x24,0x01,
    0x0E,0x31,0x33,0x2A,0x2C,0x0C,0x1B,0x14,0x2E,0x07,0x34,0x06,0x13,0x02,0x26,0x2E,
    0x2E,0x19,0x10,0x0A,0x39,0x03,0x37,0x17,0x0F,0x11,0x0B,0x0D,0x38,0x25,0x18,0x3A
};

PPU::PPU(Bus& b) : bus(b) {
    vram.resize(0x800);
    std::fill(std::begin(paletteRam), std::end(paletteRam), 0);
    std::fill(std::begin(oam), std::end(oam), 0);
    lastFrame.resize(256 * 240, 0xFF000000u);
}

void PPU::Reset() {
    std::fill(vram.begin(), vram.end(), 0);
    std::fill(std::begin(paletteRam), std::end(paletteRam), 0);
    std::fill(std::begin(oam), std::end(oam), 0);
PPUMASK = PPUSTATUS = OAMADDR = 0;
    vramAddr = vramAddrTemp = 0;
    vramLatch = false;
    ppuCycleCounter = 0;
    frameReady = true;
    scanline = 0;
    cycle = 0;

}
uint32_t PPU::RenderPixel(int x, int y) {
    // --- Nametable ---
    uint16_t baseNametableAddr = 0x2000;
    uint16_t tileIndexAddr =
        baseNametableAddr + (y / 8) * 32 + (x / 8);

    uint8_t tileIndex = vram[tileIndexAddr & 0x7FF];

    // --- Pattern table ---
    uint16_t patternTableAddr = (PPUCTRL & 0x10) ? 0x1000 : 0x0000;
    uint16_t tileDataAddr =
        patternTableAddr + tileIndex * 16 + (y % 8);

    uint8_t lowByte  = bus.ReadCHR(tileDataAddr);
    uint8_t highByte = bus.ReadCHR(tileDataAddr + 8);

    int bit = 7 - (x % 8);
    int colorIndex =
        ((highByte >> bit) & 1) << 1 |
        ((lowByte >> bit) & 1);

    // --- Attribute table ---
    uint16_t attrAddr =
        (baseNametableAddr + 0x3C0 +
         ((y / 32) * 8) + (x / 32)) & 0x7FF;

    uint8_t attr = vram[attrAddr];

    int shift =
        ((y % 32) / 16) * 4 +
        ((x % 32) / 16) * 2;

    uint8_t paletteIndex = (attr >> shift) & 0x03;

    // --- Final color (still simplified) ---
    int finalIndex = (paletteIndex * 4 + colorIndex) & 0x3F;
    uint32_t rgb = NES_COLORS[finalIndex];

    return 0xFF000000u | rgb;
}


// Advance PPU cycles; triggers a frame render when enough cycles collected
void PPU::StepCycles(uint32_t cycles) {
    for (uint32_t i = 0; i < cycles; i++) {
        // Advance cycle
        cycle++;
        if (cycle > 340) {
            cycle = 0;
            scanline++;
            if (scanline > 261) scanline = 0;
        }

    if (scanline >= 0 && scanline < 240 &&
    cycle > 0 && cycle <= 256) {

    int x = cycle - 1;
    int y = scanline;

    uint32_t color = RenderPixel(x, y);
    lastFrame[y * 256 + x] = color;
}

 // --- VBlank start ---
if (scanline == 241 && cycle == 1) {
    frameReady = true;
    PPUSTATUS |= 0x80;
    if (PPUCTRL & 0x80) {
   // ASSERT NMI
        bus.nmiLine = true;
        std::cout << "[PPU] VBlank start: NMI line asserted\n";
    }

}

// --- VBlank end ---
if (scanline == 261 && cycle == 1) {
    PPUSTATUS &= ~0xE0;   // DEASSERT NMI
}
        // End of frame
        if (scanline == 240 && cycle == 340) {

            // Reset for next frame
        }
    }
}




uint8_t PPU::ReadRegister(uint16_t reg) {
    if (bus.mapper) bus.mapper->OnPPUAddr(reg, 0);
    switch (reg) {
        case 2: { // PPUSTATUS
            uint8_t val = PPUSTATUS;

            // reading PPUSTATUS clears VBlank flag and latch
            PPUSTATUS &= ~0x80u;
            vramLatch = false;
            return val;
        }
        case 4: { // OAMDATA
            return oam[OAMADDR];
        }
        case 7: { // PPUDATA
            uint16_t addr = vramAddr & 0x3FFF;
            uint8_t data = 0;
            if (addr < 0x2000) {
                // pattern table (CHR)
                if (!bus.chrRom.empty() && addr < bus.chrRom.size()) data = bus.chrRom[addr];
            } else if (addr >= 0x2000 && addr <= 0x2FFF) {
                // nametables: mirror to 0x800
                data = vram[addr & 0x7FF];
            } else if (addr >= 0x3F00 && addr <= 0x3F1F) {
                data = paletteRam[addr & 0x1F];
            }
            // increment
            uint16_t inc = (PPUCTRL & 0x04) ? 32 : 1;
            vramAddr = (vramAddr + inc) & 0x3FFF;
            return data;
        }
        default:
            return 0;
    }
}

void PPU::WriteRegister(uint16_t reg, uint8_t val) {
    printf("PPU WRITE reg=%d val=%02X\n", reg, val);
    switch (reg) {
        case 0: // PPUCTRL
        {
            uint8_t old = PPUCTRL;
            PPUCTRL = val & 0xFF;
            if ((old & 0x80) != (PPUCTRL & 0x80)) {
                if (PPUCTRL & 0x80) std::cout << "[PPU] PPUCTRL: NMI enabled (PPUCTRL=0x" << std::hex << int(PPUCTRL) << ")" << std::dec << std::endl;
                else std::cout << "[PPU] PPUCTRL: NMI disabled (PPUCTRL=0x" << std::hex << int(PPUCTRL) << ")" << std::dec << std::endl;
            }
        }
        break;
        case 1: // PPUMASK
            PPUMASK = val;
            break;
        case 3: // OAMADDR
            OAMADDR = val;
            break;
        case 4: // OAMDATA
            oam[OAMADDR++] = val;
            break;
        case 5: // PPUSCROLL (ignored for now)
            // toggle behaviour omitted; we'll ignore scroll in phase 1
            break;
        case 6: // PPUADDR (two writes)
            if (!vramLatch) {
                vramAddrTemp = (uint16_t)(val & 0x3F) << 8; // only 14 bits but store
                vramLatch = true;
            } else {
                vramAddrTemp |= val;
                vramAddr = vramAddrTemp & 0x3FFF;
                vramLatch = false;
            }
            break;
        case 7: { // PPUDATA
            uint16_t addr = vramAddr & 0x3FFF;
            if (addr < 0x2000) {
                // writing to CHR RAM (if present)
                if (!bus.chrRom.empty() && bus.chrRom.size() > addr) {
                    bus.chrRom[addr] = val;
                }
            } else if (addr >= 0x2000 && addr <= 0x2FFF) {
                vram[addr & 0x7FF] = val;
            } else if (addr >= 0x3F00 && addr <= 0x3F1F) {
                paletteRam[addr & 0x1F] = val & 0x3F;
                static int palLog = 0;
                if (palLog < 32) {
                    std::cout << "PPU: Pal write addr=0x" << std::hex << addr << " val=0x" << int(val) << std::dec << std::endl;
                    ++palLog;
                } else if (palLog == 32) {
                    std::cout << "PPU: (further palette write logs suppressed)" << std::endl;
                    ++palLog;
                }
            }
            uint16_t inc = (PPUCTRL & 0x04) ? 32 : 1;
            vramAddr = (vramAddr + inc) & 0x3FFF;
            break;
        }
        default:
            break;
    }
}

// Return the last frame produced by StepCycles if available and clear the flag
bool PPU::PopFrame(std::vector<uint32_t>& outPixels, int& outWidth, int& outHeight) {
    if (!frameReady) return false;
    outWidth = 256;
    outHeight = 240;
    outPixels = lastFrame;
    frameReady = false;
    return true;
}

// OAM DMA: copy 256 bytes from CPU page (page<<8)
void PPU::DoOAMDMA(uint8_t page) {
    uint16_t base = static_cast<uint16_t>(page) << 8;
    // Read 256 bytes from CPU memory via bus and copy into OAM
    for (int i = 0; i < 256; ++i) {
        uint16_t addr = base + i;
        // bus.read will route to RAM/PRG/etc.
        uint8_t val = bus.read(addr);
        oam[i] = val;
    }
    // Reset OAMADDR to 0 (consistent with many emulators)
    OAMADDR = 0;
    static int dmaLogCount = 0;
    if (dmaLogCount < 8) {
        std::cout << "PPU: OAM after DMA (first 16 bytes):";
        for (int i = 0; i < 16; ++i) std::cout << " " << std::hex << int(oam[i]);
        std::cout << std::dec << std::endl;
        ++dmaLogCount;
    } else if (dmaLogCount == 8) {
        std::cout << "PPU: (further OAM DMA logs suppressed)" << std::endl;
        ++dmaLogCount;
    }
}

// Keep old helper: RenderPatternTable
bool PPU::RenderPatternTable(int tableIndex, std::vector<uint32_t>& outPixels, int& outWidth, int& outHeight, int paletteGroup) {
    const size_t PATTERN_TABLE_BYTES = 4096; // 256 tiles * 16 bytes
    size_t required = (tableIndex + 1) * PATTERN_TABLE_BYTES;
    if (bus.chrRom.size() < required) return false;

    const uint8_t* chr = bus.chrRom.data() + tableIndex * PATTERN_TABLE_BYTES;

    outWidth = 128;
    outHeight = 128;
    outPixels.assign(outWidth * outHeight, 0xFF000000u);

    // Render tiles using the selected palette group (0..3) from paletteRam and NES_COLORS
    for (int ty = 0; ty < 16; ++ty) {
        for (int tx = 0; tx < 16; ++tx) {
            int tileIndex = ty * 16 + tx;
            size_t tileBase = tileIndex * 16;
            for (int row = 0; row < 8; ++row) {
                uint8_t low = chr[tileBase + row];
                uint8_t high = chr[tileBase + row + 8];
                for (int bit = 0; bit < 8; ++bit) {
                    int shift = 7 - bit;
                    uint8_t lo = (low >> shift) & 1;
                    uint8_t hi = (high >> shift) & 1;
                    uint8_t colorIndex = lo | (hi << 1);
                    int px = tx * 8 + bit;
                    int py = ty * 8 + row;
                    // map through palette RAM (paletteGroup*4 + colorIndex) then to full NES_COLORS
                    uint8_t palEntry = paletteRam[(paletteGroup * 4) + colorIndex] & 0x3F;
                    uint32_t finalColor = NES_COLORS[palEntry % 64] | 0xFF000000u;
                    outPixels[py * outWidth + px] = finalColor;
                }
            }
        }
    }

    return true;
}
