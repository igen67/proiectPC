#pragma once
#include "memory.h"
#include "libraries.h"
#include <vector>
#include <string>
#include "input.h"

// NES memory map (simplified for now):
// $0000-$07FF: 2KB internal RAM
// $0800-$1FFF: Mirrors of $0000-$07FF
// $2000-$3FFF: PPU registers (mirrored every 8 bytes)
// $4000-$4017: APU and I/O registers
// $4018-$401F: APU and I/O functionality that is normally disabled
// $4020-$FFFF: Cartridge space (PRG-ROM, PRG-RAM, mappers)

class Bus {
public:
    static constexpr uint32_t RAM_SIZE = 0x0800; // 2KB
    static constexpr uint32_t RAM_MASK = 0x07FF;
    bool irqEnable;
    bool nmiLine = false;

    Mem ram; // Internal RAM (2KB)

    // Cartridge PRG ROM (read-only)
    std::vector<uint8_t> prgRom;

    // Cartridge CHR ROM (pattern tables)
    std::vector<uint8_t> chrRom;

    // PPU pointer (bus forwards PPU register accesses here when attached)
    class PPU* ppu = nullptr;

    // CPU pointer (for mapper IRQ requests)
    class CPU* cpu = nullptr;

    // Mapper interface attached (optional)
    class Mapper* mapper = nullptr;

    // Input / controller state
    class Input input;

    // Cartridge mirroring (from iNES flags)
    bool mirrorVertical = false;

    // OAM DMA state: when a CPU writes to $4014, emulator may need to stall CPU cycles
    bool oamDmaActive = false;
    uint8_t oamDmaPage = 0;
    uint32_t oamDmaCycles = 0;
    // Per-byte DMA state
    uint16_t oamDmaIndex = 0;
    bool oamDmaDummy = true; // first dummy cycle behavior

    // Host-facing helper to set controller button bits (bit0=A, bit1=B, bit2=Select, bit3=Start, bit4=Up, bit5=Down, bit6=Left, bit7=Right)
    void SetControllerButtons(uint8_t buttons) { input.SetButtons(0, buttons); }

    // Attach CPU so mappers can request IRQs
    void AttachCPU(class CPU* c) { cpu = c; }

    // Attach a PPU instance to the bus so PPU registers can be forwarded
    void AttachPPU(class PPU* p) { ppu = p; }

    // Register a mapper instance
    void AttachMapper(class Mapper* m) { mapper = m; }

    // Read CHR through mapper if present (used by PPU)
    uint8_t ReadCHR(uint16_t addr) const;

    // Notify mapper that PPU read occurred at addr (for MMC3 A12 detection)
    void NotifyPPUAddr(uint16_t addr);

    // Initialize bus and RAM
    Bus();

    // Read a byte from the bus
    uint8_t read(uint16_t addr) const;

    // Write a byte to the bus
    void write(uint16_t addr, uint8_t value);

    // Load PRG ROM (and CHR for iNES) from a file (iNES .nes or raw PRG ROM).
    // Returns true on success.
    bool LoadPRGFromFile(const std::string& filename);

    // Utility: clear cartridge data
    void UnloadCartridge() { prgRom.clear(); chrRom.clear(); }

};
