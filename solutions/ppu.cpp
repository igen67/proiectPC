#include "headers/ppu.h"
#include "headers/bus.h"
#include "headers/cpu.h"
#include "headers/mapper.h"
#include <cstring>
#include <algorithm>

// Basic NES palette for visualization (64 colors approximated)
static const uint32_t NES_COLORS[64] = {
    0xFF757575u,0xFF271B8Fu,0xFF0000ABu,0xFF47009Fu,0xFF8F0077u,0xFFA7003Bu,0xFFA70000u,0xFF7F0B00u,
    0xFF432F00u,0xFF004700u,0xFF005100u,0xFF003F17u,0xFF1B3F5Fu,0xFF000000u,0xFF000000u,0xFF000000u,
    0xFFBCBCBCu,0xFF0073EFu,0xFF233BEFu,0xFF8300F3u,0xFFBF00BFu,0xFFE7005Bu,0xFFDB2B00u,0xFFCB4F0Fu,
    0xFF8B7300u,0xFF004700u,0xFF004F00u,0xFF00432Bu,0xFF00407Fu,0xFF000000u,0xFF000000u,0xFF000000u,
    0xFFF7F7F7u,0xFF3FBFFFu,0xFF5F97FFu,0xFFA78BFDu,0xFFF77BFFu,0xFFFF77B7u,0xFFFF7763u,0xFFFF9B3Fu,
    0xFFF3BF3Fu,0xFF56DB57u,0xFF4DEB8Bu,0xFF60EFD5u,0xFFB3E7EFu,0xFF000000u,0xFF000000u,0xFF000000u,
    0xFFFFFFFFu,0xFFABE7FFu,0xFFC7D7FFu,0xFFF7C7FFu,0xFFFFC7FFu,0xFFFFC7DBu,0xFFFFC7B7u,0xFFFFDBABu,
    0xFFF7E7A3u,0xFFDBF7BDu,0xFFBFF3BFu,0xFFBFF3DFu,0xFFE8F7FFu,0xFF000000u,0xFF000000u,0xFF000000u
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
    PPUCTRL = PPUMASK = PPUSTATUS = OAMADDR = 0;
    vramAddr = vramAddrTemp = 0;
    vramLatch = false;
    ppuCycleCounter = 0;
    frameReady = false;
}

// Advance PPU cycles; triggers a frame render when enough cycles collected
void PPU::StepCycles(uint32_t cycles) {
    if (cycles == 0) return;
    const uint32_t CYCLES_PER_FRAME = 341u * 262u; // 89342
    ppuCycleCounter += cycles;
    if (ppuCycleCounter >= CYCLES_PER_FRAME) {
        // Render and wrap counter (support multiple frames being advanced at once)
        ppuCycleCounter %= CYCLES_PER_FRAME;
        // Produce a frame into lastFrame
        int w=0,h=0;
        RenderFrame(lastFrame, w, h);
        frameReady = true;
        // set VBlank flag
        bool wasVBlank = (PPUSTATUS & 0x80) != 0;
        PPUSTATUS |= 0x80;
        // Trigger NMI on entering VBlank if enabled in PPUCTRL bit 7
        if (!wasVBlank && (PPUCTRL & 0x80) && bus.ppu /* placeholder to silence unused */) {
            if (bus.cpu) {
                bus.cpu->NMIRequested = true;
            }
        }
    }
}

uint8_t PPU::ReadRegister(uint16_t reg) {
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
            vramAddr = (vramAddr + inc) & 0x7FFF;
            return data;
        }
        default:
            return 0;
    }
}

void PPU::WriteRegister(uint16_t reg, uint8_t val) {
    switch (reg) {
        case 0: // PPUCTRL
            PPUCTRL = val;
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
                vramAddr = vramAddrTemp & 0x7FFF;
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
            vramAddr = (vramAddr + inc) & 0x7FFF;
            break;
        }
        default:
            break;
    }
}

// Render a full 256x240 frame into outPixels. Uses nametable at $2000 and list simplifications.
bool PPU::RenderFrame(std::vector<uint32_t>& outPixels, int& outWidth, int& outHeight) {
    outWidth = 256;
    outHeight = 240;
    outPixels.assign(outWidth * outHeight, 0xFF000000u);

    // We'll record background color indices so sprite priority (behind/background) can be respected
    std::vector<uint8_t> bgIndex(outWidth * outHeight, 0);

    // Use nametable base at $2000 (NT0). We'll respect mirrorVertical from the bus when mapping other nametables.
    uint16_t ntBase = 0x2000;

    // For each tile on the screen (32x30 tiles)
    for (int ty = 0; ty < 30; ++ty) {
        for (int tx = 0; tx < 32; ++tx) {
            uint16_t tileIndex = vram[(ntBase - 0x2000) + ty * 32 + tx];
            // attribute byte index computation
            int attrX = tx / 4;
            int attrY = ty / 4;
            uint8_t attr = vram[0x3C0 + attrY * 8 + attrX];
            int localTx = (tx % 4) / 2; // 0 or 1
            int localTy = (ty % 4) / 2; // 0 or 1
            int shift = (localTy * 2 + localTx) * 2;
            uint8_t paletteHighBits = (attr >> shift) & 0x3;

            int tileBase = tileIndex * 16; // in CHR
            // background pattern table: choose 0 or 1 from PPUCTRL bit 4
            int patternBase = (PPUCTRL & 0x10) ? 0x1000 : 0x0000;
            const uint8_t* chr = nullptr;
            if (!bus.chrRom.empty()) chr = bus.chrRom.data() + patternBase;

            for (int row = 0; row < 8; ++row) {
                uint16_t addrLow = patternBase + tileBase + row;
                uint16_t addrHigh = patternBase + tileBase + row + 8;
                uint8_t low = 0, high = 0;
                // Notify mapper/Bus about PPU CHR reads so mappers (MMC3) can detect A12 rising
                bus.NotifyPPUAddr(addrLow);
                bus.NotifyPPUAddr(addrHigh);
                if (bus.mapper) {
                    low = bus.mapper->CHRRead(addrLow);
                    high = bus.mapper->CHRRead(addrHigh);
                } else if (chr) {
                    low = chr[tileBase + row];
                    high = chr[tileBase + row + 8];
                }
                for (int col = 0; col < 8; ++col) {
                    int bit = 7 - col;
                    uint8_t lo = (low >> bit) & 1;
                    uint8_t hi = (high >> bit) & 1;
                    uint8_t colorIndex = lo | (hi << 1);
                    // paletteRam index: 0x3F00 + paletteHighBits*4 + colorIndex
                    uint8_t palIndex = paletteRam[(paletteHighBits * 4) + colorIndex] & 0x3F;
                    uint32_t finalColor = NES_COLORS[palIndex % 64];
                    int px = tx * 8 + col;
                    int py = ty * 8 + row;
                    if (px < outWidth && py < outHeight) {
                        outPixels[py * outWidth + px] = finalColor | 0xFF000000u;
                        bgIndex[py * outWidth + px] = colorIndex; // 0 means background transparent
                    }
                }
            }
        }
    }

    // --- Sprite rendering (OAM) ---
    // Each sprite: Y, tile, attr, X (Y is stored as top-1)
    int spriteHeight = (PPUCTRL & 0x20) ? 16 : 8;
    // Draw sprites so that lower OAM indices have higher priority -> draw in reverse so index 0 ends up on top
    for (int si = 63; si >= 0; --si) {
        int base = si * 4;
        int spriteY = static_cast<int>(oam[base + 0]) + 1; // stored as top-1
        int tile = oam[base + 1];
        uint8_t attr = oam[base + 2];
        int spriteX = static_cast<int>(oam[base + 3]);

        bool flipV = (attr & 0x80) != 0;
        bool flipH = (attr & 0x40) != 0;
        bool behindBg = (attr & 0x20) != 0; // sprite behind background if set
        int paletteSelect = attr & 0x3; // selects sprite palette at $3F10 + palette*4

        // Determine pattern table base for sprites (8x8 uses PPUCTRL bit 3; 8x16 builds from tile LSB)
        for (int row = 0; row < spriteHeight; ++row) {
            int srcRow = flipV ? (spriteHeight - 1 - row) : row;
            int patternBase = 0;
            int tileIndex = tile;
            if (spriteHeight == 8) {
                patternBase = (PPUCTRL & 0x08) ? 0x1000 : 0x0000; // sprite table select
                int tileBase = tileIndex * 16;
                uint16_t addrLow = patternBase + tileBase + srcRow;
                uint16_t addrHigh = patternBase + tileBase + srcRow + 8;
                bus.NotifyPPUAddr(addrLow);
                bus.NotifyPPUAddr(addrHigh);
                uint8_t low = 0, high = 0;
                if (bus.mapper) {
                    low = bus.mapper->CHRRead(addrLow);
                    high = bus.mapper->CHRRead(addrHigh);
                } else if (!bus.chrRom.empty()) {
                    const uint8_t* chr = bus.chrRom.data() + patternBase;
                    low = chr[tileBase + srcRow];
                    high = chr[tileBase + srcRow + 8];
                }

                for (int col = 0; col < 8; ++col) {
                    int srcCol = flipH ? col : (7 - col);
                    int bit = srcCol;
                    uint8_t lo = (low >> bit) & 1;
                    uint8_t hi = (high >> bit) & 1;
                    uint8_t colorIndex = lo | (hi << 1);
                    if (colorIndex == 0) continue; // transparent
                    int px = spriteX + col;
                    int py = spriteY + row;
                    if (px < 0 || px >= outWidth || py < 0 || py >= outHeight) continue;
                    // If sprite behind background and background pixel non-zero, skip drawing
                    if (behindBg && bgIndex[py * outWidth + px] != 0) continue;
                    // Palette index at $3F10 + paletteSelect*4 + colorIndex
                    uint8_t palIndex = paletteRam[0x10 + paletteSelect * 4 + colorIndex] & 0x3F;
                    uint32_t finalColor = NES_COLORS[palIndex % 64] | 0xFF000000u;

                    // Sprite-0 hit detection: if sprite 0 overlaps non-zero background
                    if (si == 0 && bgIndex[py * outWidth + px] != 0) {
                        PPUSTATUS |= 0x40; // set sprite 0 hit
                    }

                    outPixels[py * outWidth + px] = finalColor;
                }
            } else {
                // 8x16 mode
                // pattern table is selected by tile's low bit
                patternBase = (tileIndex & 0x1) ? 0x1000 : 0x0000;
                int baseTile = tileIndex & 0xFE; // even tile selects top
                int tileNum = (srcRow < 8) ? baseTile : (baseTile + 1);
                int rowInTile = (srcRow < 8) ? srcRow : (srcRow - 8);
                int tileBase = tileNum * 16;
                uint16_t addrLow = patternBase + tileBase + rowInTile;
                uint16_t addrHigh = patternBase + tileBase + rowInTile + 8;
                bus.NotifyPPUAddr(addrLow);
                bus.NotifyPPUAddr(addrHigh);
                uint8_t low = 0, high = 0;
                if (bus.mapper) {
                    low = bus.mapper->CHRRead(addrLow);
                    high = bus.mapper->CHRRead(addrHigh);
                } else if (!bus.chrRom.empty()) {
                    const uint8_t* chr = bus.chrRom.data() + patternBase;
                    low = chr[tileBase + rowInTile];
                    high = chr[tileBase + rowInTile + 8];
                }

                for (int col = 0; col < 8; ++col) {
                    int srcCol = flipH ? col : (7 - col);
                    int bit = srcCol;
                    uint8_t lo = (low >> bit) & 1;
                    uint8_t hi = (high >> bit) & 1;
                    uint8_t colorIndex = lo | (hi << 1);
                    if (colorIndex == 0) continue;
                    int px = spriteX + col;
                    int py = spriteY + row;
                    if (px < 0 || px >= outWidth || py < 0 || py >= outHeight) continue;
                    if (behindBg && bgIndex[py * outWidth + px] != 0) continue;
                    uint8_t palIndex = paletteRam[0x10 + paletteSelect * 4 + colorIndex] & 0x3F;
                    uint32_t finalColor = NES_COLORS[palIndex % 64] | 0xFF000000u;
                    if (si == 0 && bgIndex[py * outWidth + px] != 0) {
                        PPUSTATUS |= 0x40;
                    }
                    outPixels[py * outWidth + px] = finalColor;
                }
            }
        }
    }

    // Do not modify frameReady when forcing a render; PopFrame is used to consume completed frames.
    return true;
}

// Return the last frame produced by StepCycles if available and clear the flag
bool PPU::PopFrame(std::vector<uint32_t>& outPixels, int& outWidth, int& outHeight) {
    if (!frameReady) return false;
    outWidth = 256;
    outHeight = 240;
    outPixels = lastFrame;
    frameReady = false;
    // clear VBlank (mimic read of PPUSTATUS behavior)
    PPUSTATUS &= ~0x80u;
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
