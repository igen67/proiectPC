#include "headers/ppu.h"
#include "headers/bus.h"
#include "headers/cpu.h"
#include "headers/mapper.h"
#include <cstring>
#include <algorithm>
#include <cstdio>
bool nmiOccurred = false;

namespace {
void LogPPUStatus(const char* label, uint8_t value, int scanline, int cycle, int pc, bool force) {
    static uint8_t lastValue = 0xFF;
    if (!force && value == lastValue) return;
    lastValue = value;
    if (scanline >= 0 && cycle >= 0) {
        if (pc >= 0) {
            std::fprintf(stderr, "[PPU] %s: PPUSTATUS=0x%02X (s=%d c=%d pc=0x%04X)\n", label, value, scanline, cycle, pc);
        } else {
            std::fprintf(stderr, "[PPU] %s: PPUSTATUS=0x%02X (s=%d c=%d)\n", label, value, scanline, cycle);
        }
    } else {
        if (pc >= 0) {
            std::fprintf(stderr, "[PPU] %s: PPUSTATUS=0x%02X (pc=0x%04X)\n", label, value, pc);
        } else {
            std::fprintf(stderr, "[PPU] %s: PPUSTATUS=0x%02X\n", label, value);
        }
    }
}
}
const uint32_t NES_COLORS[64] = {
    0xFF757575, 0xFF271B8F, 0xFF0000AB, 0xFF47009F, 0xFF8F0077, 0xFFAB0013, 0xFFA70000, 0xFF7F0B00,
    0xFF432F00, 0xFF004700, 0xFF005100, 0xFF003F17, 0xFF1B3F5F, 0xFF000000, 0xFF000000, 0xFF000000,
    0xFFBCBCBC, 0xFF0073EF, 0xFF233BEF, 0xFF8300F3, 0xFFBF00BF, 0xFFE7005B, 0xFFDB2B00, 0xFFCB4F0F,
    0xFF8B7300, 0xFF009700, 0xFF00AB00, 0xFF00933B, 0xFF00838B, 0xFF000000, 0xFF000000, 0xFF000000,
    0xFFFFFFFF, 0xFF3FBFFF, 0xFF5F97FF, 0xFFA78BFD, 0xFFF77BFF, 0xFFFF77B7, 0xFFFF7763, 0xFFFF9B3B,
    0xFFF3BF3F, 0xFF4FFB73, 0xFF66FF66, 0xFF4DE09E, 0xFF4CD7D7, 0xFF000000, 0xFF000000, 0xFF000000,
    0xFFFFFEFF, 0xFFA7E7FF, 0xFFC7D7FF, 0xFFD7CBFF, 0xFFFFC7FF, 0xFFFFC7DB, 0xFFFFBFA3, 0xFFFFDBAB,
    0xFFFFE7A3, 0xFFE3FFCF, 0xFFABF3BF, 0xFFB3FFCF, 0xFF9FFFF3, 0xFF000000, 0xFF000000, 0xFF000000
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
    LogPPUStatus("Reset (before)", PPUSTATUS, -1, -1, -1, true);
    PPUMASK = PPUSTATUS = OAMADDR = 0;
    vramAddr = vramAddrTemp = 0;
    writeToggle = false;
    fineX = 0;
    scrollCoarseX = 0;
    scrollCoarseY = 0;
    scrollFineY = 0;
    renderAddr = 0;
    ppuCycleCounter = 0;
    frameReady = false ;
    // Start at pre-render scanline so v/t copies run before first visible line.
    scanline = 261;
    cycle = 0;

}
uint16_t PPU::MapNametable(uint16_t addr) const {
    addr &= 0x0FFF; // $2000–$2FFF

    uint16_t table  = addr / 0x0400; // 0–3
    uint16_t offset = addr & 0x03FF;

    Mirroring mirroring = bus.mapper
        ? bus.mapper->GetMirroring()
        : (bus.mirrorVertical ? Mirroring::Vertical : Mirroring::Horizontal);

    switch (mirroring) {
        case Mirroring::Vertical:
            return (table & 1) * 0x0400 + offset;

        case Mirroring::Horizontal:
            return (table >> 1) * 0x0400 + offset;

        case Mirroring::SingleScreenA:
            return offset;

        case Mirroring::SingleScreenB:
            return 0x0400 + offset;

        case Mirroring::FourScreen:
            return addr; // mapper handles extra VRAM

        default:
            return offset;
    }
}
uint32_t PPU::RenderPixel(int x, int y) {
    bool bgEnabled = (PPUMASK & 0x08) != 0;
    bool sprEnabled = (PPUMASK & 0x10) != 0;

    uint8_t bgColorIndex = 0;
    uint8_t bgPaletteIndex = 0;
    if (bgEnabled && !(x < 8 && (PPUMASK & 0x02) == 0)) {
        uint16_t coarseX = renderAddr & 0x001F;
        uint16_t coarseY = (renderAddr >> 5) & 0x001F;
        uint16_t fineY = (renderAddr >> 12) & 0x0007;

        uint16_t ntIndex = (renderAddr >> 10) & 0x0003;
        uint16_t baseNametableAddr = 0x2000 + (ntIndex * 0x0400);
        uint16_t tileIndexAddr =
            baseNametableAddr + coarseY * 32 + coarseX;

        uint16_t ntAddr = MapNametable(tileIndexAddr);
        uint8_t tileIndex = vram[ntAddr];

        uint16_t patternTableAddr = (PPUCTRL & 0x10) ? 0x1000 : 0x0000;
        uint16_t tileDataAddr =
            patternTableAddr + tileIndex * 16 + fineY;

        bus.NotifyPPUAddr(tileDataAddr);
        bus.NotifyPPUAddr(tileDataAddr + 8);
        uint8_t lowByte  = bus.ReadCHR(tileDataAddr);
        uint8_t highByte = bus.ReadCHR(tileDataAddr + 8);


        int bit = 7 - ((x + fineX) & 7);
        bgColorIndex =
            ((highByte >> bit) & 1) << 1 |
            ((lowByte >> bit) & 1);

        uint16_t attrAddr =
            baseNametableAddr + 0x3C0 +
            ((coarseY >> 2) * 8) + (coarseX >> 2);

        uint16_t attrNt = MapNametable(attrAddr);
        uint8_t attr = vram[attrNt];


        int quadrantY = (coarseY & 0x02) ? 1 : 0;
        int quadrantX = (coarseX & 0x02) ? 1 : 0;
        int shift = (quadrantY * 2 + quadrantX) * 2;

        bgPaletteIndex = (attr >> shift) & 0x03;
    }

    bool spritePixel = false;
    uint8_t spriteColorIndex = 0;
    uint8_t spritePaletteIndex = 0;
    bool spriteBehind = false;

    if (sprEnabled && !(x < 8 && (PPUMASK & 0x04) == 0)) {
        int spriteHeight = (PPUCTRL & 0x20) ? 16 : 8;
        uint16_t spriteTable = (PPUCTRL & 0x08) ? 0x1000 : 0x0000;
        for (int i = 0; i < 64; ++i) {
            uint8_t sy = oam[i * 4 + 0];
            uint8_t tile = oam[i * 4 + 1];
            uint8_t attr = oam[i * 4 + 2];
            uint8_t sx = oam[i * 4 + 3];

            int spriteY = int(sy) + 1;
            if (y < spriteY || y >= spriteY + spriteHeight) continue;
            if (x < sx || x >= sx + 8) continue;

            int row = y - spriteY;
            bool flipV = (attr & 0x80) != 0;
            bool flipH = (attr & 0x40) != 0;
            spriteBehind = (attr & 0x20) != 0;
            if (flipV) row = spriteHeight - 1 - row;

            uint16_t tileAddr = 0;
            if (spriteHeight == 16) {
                uint16_t table = (tile & 0x01) ? 0x1000 : 0x0000;
                uint8_t tileIndex = tile & 0xFE;
                if (row >= 8) {
                    tileIndex += 1;
                    row -= 8;
                }
                tileAddr = table + tileIndex * 16 + row;
            } else {
                tileAddr = spriteTable + tile * 16 + row;
            }

            int col = x - sx;
            if (flipH) col = 7 - col;
            int bit = 7 - col;

            bus.NotifyPPUAddr(tileAddr);
            bus.NotifyPPUAddr(tileAddr + 8);
            uint8_t low = bus.ReadCHR(tileAddr);
            uint8_t high = bus.ReadCHR(tileAddr + 8);
            uint8_t color = ((high >> bit) & 1) << 1 | ((low >> bit) & 1);
            if (color == 0) continue;

            spritePixel = true;
            spriteColorIndex = color;
            spritePaletteIndex = attr & 0x03;
            break;
        }
    }

    uint8_t palEntry = 0;
    if (spritePixel && (bgColorIndex == 0 || !spriteBehind)) {
        int palIndex = 0x10 + spritePaletteIndex * 4 + spriteColorIndex;
        palEntry = paletteRam[palIndex & 0x1F] & 0x3F;
    } else {
        int palIndex = (bgColorIndex == 0) ? 0 : (bgPaletteIndex * 4 + bgColorIndex);
        palEntry = paletteRam[palIndex & 0x1F] & 0x3F;
    }

    uint32_t finalColor = NES_COLORS[palEntry % 64];
    return finalColor;

}



// Advance PPU cycles; triggers a frame render when enough cycles collected
void PPU::StepCycles(uint32_t cycles) {
    for (uint32_t i = 0; i < cycles; i++) {
        auto incrementX = [this]() {
            if ((renderAddr & 0x001F) == 31) {
                renderAddr &= ~0x001F;
                renderAddr ^= 0x0400;
            } else {
                renderAddr += 1;
            }
        };
        auto incrementY = [this]() {
            if ((renderAddr & 0x7000) != 0x7000) {
                renderAddr += 0x1000;
            } else {
                renderAddr &= ~0x7000;
                uint16_t y = (renderAddr & 0x03E0) >> 5;
                if (y == 29) {
                    y = 0;
                    renderAddr ^= 0x0800;
                } else if (y == 31) {
                    y = 0;
                } else {
                    y += 1;
                }
                renderAddr = (renderAddr & ~0x03E0) | (y << 5);
            }
        };
        auto copyHorizontal = [this]() {
            renderAddr = (renderAddr & ~0x041F) | (vramAddrTemp & 0x041F);
        };
        auto copyVertical = [this]() {
            renderAddr = (renderAddr & ~0x7BE0) | (vramAddrTemp & 0x7BE0);
        };

        bool rendering = (PPUMASK & 0x18) != 0;
        if (scanline < 240 && cycle >= 1 && cycle <= 256) {
            int x = cycle - 1;
            int y = scanline;
            if (rendering) {
                lastFrame[y * 256 + x] = RenderPixel(x, y);
                if ((cycle % 8) == 0 && cycle != 256) {
                    incrementX();
                }
                if (cycle == 256) {
                    incrementY();
                }
            } else {
                lastFrame[y * 256 + x] = 0xFF000000u;
            }
        }
        if (rendering) {
            if ((scanline < 240 || scanline == 261) && cycle == 257) {
                copyHorizontal();
            }
            if (scanline == 261 && cycle >= 280 && cycle <= 304) {
                copyVertical();
            }
        }
        

        // VBlank start
        if (scanline == 241 && cycle == 1) {
    int pc = bus.cpu ? static_cast<int>(bus.cpu->PC) : -1;
    LogPPUStatus("VBlank start (before)", PPUSTATUS, scanline, cycle, pc, false);
    PPUSTATUS |= 0x80;

    if ((PPUCTRL & 0x80) && !nmiOccurred) {
        bus.nmiLine = true;
        nmiOccurred = true;
        std::cout << "NMI at s=" << scanline
          << " c=" << cycle << "\n";
    }
}

        // Pre-render line
        if (scanline == 261 && cycle == 1) {
            // pre-render line: clear VBlank and secondary flags
                    int pc = bus.cpu ? static_cast<int>(bus.cpu->PC) : -1;
                    LogPPUStatus("Pre-render clear (before)", PPUSTATUS, scanline, cycle, pc, false);
                    PPUSTATUS &= ~0x80;
                     PPUSTATUS &= ~0x40;
                 PPUSTATUS &= ~0x20;
               nmiOccurred = false;
        }

        // Frame finished
        if (scanline == 241 && cycle == 0) {
            frameReady = true;
        }
        cycle++;
        if (cycle == 341) {
            cycle = 0;
            scanline++;
            if (scanline == 262)
                scanline = 0;
        }
    }
}




uint8_t PPU::ReadRegister(uint16_t reg) {
    reg &= 7;
    switch (reg) {
        case 2: { // PPUSTATUS
            uint8_t ret = PPUSTATUS;
            int pc = bus.cpu ? static_cast<int>(bus.cpu->PC) : -1;
            LogPPUStatus("Read $2002 (before)", PPUSTATUS, scanline, cycle, pc, false);
            PPUSTATUS &= ~0x80;
            writeToggle = false;
            return ret;
        }
        case 4: { // OAMDATA
            return oam[OAMADDR];
        }
        case 7: { // PPUDATA
            uint16_t addr = vramAddr & 0x3FFF;
            uint8_t ret = 0;
            // PPUDATA is buffered for reads from $0000-$3EFF
            if (addr >= 0x3F00 && addr <= 0x3FFF) {
                uint8_t palIndex = addr & 0x1F;
                if ((palIndex & 0x13) == 0x10) {
                    palIndex &= ~0x10;
                }
                ret = paletteRam[palIndex];
            } else {
                // Return buffered value and refill buffer with current memory read
                ret = readBuffer;
                if (addr < 0x2000) {
                    // pattern table (CHR)
                    bus.NotifyPPUAddr(addr);
                    readBuffer = bus.ReadCHR(addr);
                } else if (addr >= 0x2000 && addr <= 0x3EFF) {
                    // $3000-$3EFF mirrors $2000-$2EFF
                    uint16_t nt = MapNametable(addr & 0x2FFF);
                    readBuffer = vram[nt];
                } else {
                    readBuffer = 0;
                }
            }
            // increment vramAddr
                uint16_t inc = (PPUCTRL & 0x04) ? 32 : 1;
            vramAddr = (vramAddr + inc) & 0x3FFF;
            return ret;
        }
        default:
            return 0;
    }
}

void PPU::WriteRegister(uint16_t reg, uint8_t val) {
    switch (reg) {
        case 0: // PPUCTRL
        {
            PPUCTRL = val;
            vramAddrTemp = (vramAddrTemp & 0xF3FF) | ((val & 0x03) << 10);
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
        case 5: // PPUSCROLL
            if (!writeToggle) {
                // first write: coarse X (t: bits 0-4) and fine X (x)
                fineX = val & 0x07;
                scrollCoarseX = (val >> 3) & 0x1F;
                vramAddrTemp = (vramAddrTemp & 0xFFE0) | (uint16_t)(val >> 3);
                writeToggle = true;
            } else {
                // second write: coarse Y (t: bits 5-9) and fine Y (t: bits 12-14)
                uint16_t coarseY = (uint16_t)(val >> 3) & 0x1F;
                uint16_t fineY = (uint16_t)(val & 0x07);
                scrollCoarseY = coarseY;
                scrollFineY = fineY;
                vramAddrTemp = (vramAddrTemp & 0x8C1F) | (coarseY << 5) | (fineY << 12);
                writeToggle = false;
            }
            break;
        case 6: // PPUADDR (two writes)
            if (!writeToggle) {
                // first write: high 6 bits of vram address
                vramAddrTemp = (vramAddrTemp & 0x00FF) | ((val & 0x3F) << 8);
                writeToggle = true;
            } else {
                // second write: low 8 bits, then copy t -> v
                vramAddrTemp |= val;
                vramAddr = vramAddrTemp & 0x3FFF;
                writeToggle = false;
            }
            break;
        case 7: { // PPUDATA
            uint16_t addr = vramAddr & 0x3FFF;
            if (addr < 0x2000) {
                // writing to CHR RAM (if present)
                bus.WriteCHR(addr, val);
            } else if (addr >= 0x2000 && addr <= 0x3EFF) {
                // $3000-$3EFF mirrors $2000-$2EFF
                uint16_t ntAddr = addr & 0x2FFF;
                uint16_t nt = MapNametable(ntAddr);
                vram[nt] = val;
            } else if (addr >= 0x3F00 && addr <= 0x3FFF) {
                uint16_t palAddr = addr & 0x1F;
                if ((palAddr & 0x13) == 0x10) {
                        palAddr &= ~0x10;
                        }
                paletteRam[palAddr] = val & 0x3F;
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
    outPixels = std::move(lastFrame);
    lastFrame.resize(256 * 240);
    frameReady = false;
    return true;
}

// OAM DMA: copy 256 bytes from CPU page (page<<8)
void PPU::DoOAMDMA(uint8_t page) {
    // Legacy helper that performs a whole-frame copy; kept for compatibility
    uint16_t base = static_cast<uint16_t>(page) << 8;
    for (int i = 0; i < 256; ++i) {
        uint16_t addr = base + i;
        uint8_t val = bus.read(addr);
        oam[i] = val;
    }
    OAMADDR = 0;
}

// Write a single byte into OAM at index (used by cycle-accurate DMA)
void PPU::WriteOAMByte(uint16_t index, uint8_t value) {
    if (index < 256) oam[index] = value;
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
