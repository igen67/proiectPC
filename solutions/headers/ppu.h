#pragma once
#include <vector>
#include <cstdint>

class Bus;

class PPU {
public:
    explicit PPU(Bus& bus);

    // Reset PPU state
    void Reset();

    // Handle PPU cycles advanced by the CPU (ppuCycles = cpuCycles * 3)
    void StepCycles(uint32_t ppuCycles);

    // Read/Write PPU registers (reg = 0..7 correspond to $2000..$2007)
    uint8_t ReadRegister(uint16_t reg);
    void WriteRegister(uint16_t reg, uint8_t val);

    // Force render of a frame from current memory (useful for GUI immediate view)
    bool RenderFrame(std::vector<uint32_t>& outPixels, int& outWidth, int& outHeight);

    // Pop a completed frame that was produced by StepCycles. Returns true if a frame was available.
    bool PopFrame(std::vector<uint32_t>& outPixels, int& outWidth, int& outHeight);

    // Keep existing pattern table helper (paletteGroup selects which 4-color palette to use; 0..3)
    bool RenderPatternTable(int tableIndex, std::vector<uint32_t>& outPixels, int& outWidth, int& outHeight, int paletteGroup = 0);

    // Debug getters (inspect internal VRAM/OAM/palette from the GUI)
    const uint8_t* GetVRAM() const { return vram.data(); }
    size_t GetVRAMSize() const { return vram.size(); }
    const uint8_t* GetOAM() const { return oam; }
    const uint8_t* GetPaletteRam() const { return paletteRam; }
    uint8_t GetPPUCTRL() const { return PPUCTRL; }
    uint8_t GetPPUMASK() const { return PPUMASK; }
    uint8_t GetPPUSTATUS() const { return PPUSTATUS; }
    int GetSpriteHeight() const { return (PPUCTRL & 0x20) ? 16 : 8; }

    // OAM DMA: copy 256 bytes from CPU page (value<<8) into OAM
    void DoOAMDMA(uint8_t page);

private:
    Bus& bus;

    // Internal VRAM (2KB) for name tables (mirrored as appropriate)
    std::vector<uint8_t> vram; // 2KB

    // Palette RAM (32 bytes)
    uint8_t paletteRam[32];

    // OAM (sprites) - simplified 256 bytes for now
    uint8_t oam[256];

    // Registers
    uint8_t PPUCTRL = 0;
    uint8_t PPUMASK = 0;
    uint8_t PPUSTATUS = 0;
    uint8_t OAMADDR = 0;

    // VRAM address latch and temp
    uint16_t vramAddr = 0;
    uint16_t vramAddrTemp = 0;
    bool vramLatch = false; // two-write toggle for $2005/$2006

    // PPU cycle/frame counters
    uint32_t ppuCycleCounter = 0;
    bool frameReady = false;
    int scanline = 0;
    int cycle = 0;

    // Last rendered full frame (256x240) RGBA32
    std::vector<uint32_t> lastFrame;
};
