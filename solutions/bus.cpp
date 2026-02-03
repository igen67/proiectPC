#pragma once
#include "headers/memory.h"
#include "headers/bus.h"
#include "headers/ppu.h"
#include "headers/mapper.h"

#include <fstream>
#include <iostream>

Bus::Bus() {
    ram.Initialise();
    prgRom.clear();
    chrRom.clear();
    ppu = nullptr;
    cpu = nullptr;
    mapper = nullptr;
    mirrorVertical = false;
    irqEnable = false;
}


uint8_t Bus::read(uint16_t addr) const {
    // RAM and mirrors
    if (addr <= 0x1FFF) {
        return ram.Data[addr & RAM_MASK];
    }

    // PPU registers mirrored every 8 bytes: $2000-$3FFF
    if (addr >= 0x2000 && addr <= 0x3FFF && ppu) {
        uint16_t reg = (addr - 0x2000) & 0x7;
        return ppu->ReadRegister(reg);
    }

    // Mapper-aware PRG area
    if (mapper) {
        // Mapper handles CPU read mapping for $6000-$FFFF
        if (addr >= 0x6000) return mapper->CPURead(addr);
    }

    // PRG-ROM: fallback mapping for simple ROMs (NROM)
 if (!prgRom.empty() && addr >= 0x8000) {
    if (prgRom.size() == 0x4000) {
        // 16KB PRG: mirror
        return prgRom[(addr - 0x8000) & 0x3FFF];
    } else {
        // 32KB PRG
        return prgRom[addr - 0x8000];
    }
}

    // For other addresses, return underlying RAM image (useful for tests)
    return ram.Data[addr];
}

uint8_t Bus::ReadCHR(uint16_t addr) const {
    if (mapper) return mapper->CHRRead(addr);
    if (!chrRom.empty() && addr < chrRom.size()) return chrRom[addr];
    return 0;
}

void Bus::NotifyPPUAddr(uint16_t addr) {
    // Allow mapper to detect PPU address access (for MMC3 A12 edge detection)
    if (mapper) mapper->OnPPUAddr(addr, 0);
}

void Bus::write(uint16_t addr, uint8_t value) {
    // RAM and mirrors
    if (addr <= 0x1FFF) {
        ram.Data[addr & RAM_MASK] = value;
        return;
    }

    // PPU registers mirrored every 8 bytes: $2000-$3FFF
    if (addr >= 0x2000 && addr <= 0x3FFF && ppu) {
        static int ppuWriteLogCount = 0;
        if (ppuWriteLogCount < 64) {
            std::cout << "Bus: write to PPU addr=0x" << std::hex << addr << " val=0x" << int(value) << std::dec << std::endl;
            ++ppuWriteLogCount;
        } else if (ppuWriteLogCount == 64) {
            std::cout << "Bus: (further PPU writes suppressed)" << std::endl;
            ++ppuWriteLogCount;
        }
        Word reg = (addr - 0x2000) & 0x7;
        ppu->WriteRegister(reg, value);
        return;
    }

    // Mapper-aware PRG area handling
    if (mapper) {
        if (addr >= 0x6000) {
            mapper->CPUWrite(addr, value);
            return;
        }
    }

    // OAM DMA write (0x4014): copy 256 bytes from CPU page (value<<8) into PPU OAM
    if (addr == 0x4014 && ppu) {
        static int oamDmaLogCount = 0;
        if (oamDmaLogCount < 8) {
            std::cout << "Bus: OAMDMA page=0x" << std::hex << int(value) << std::dec << std::endl;
            ++oamDmaLogCount;
        } else if (oamDmaLogCount == 8) {
            std::cout << "Bus: (further OAMDMA logs suppressed)" << std::endl;
            ++oamDmaLogCount;
        }
        ppu->DoOAMDMA(value);
        return;
    }

    // PRG-ROM area is read-only; ignore writes (fallback for NROM)
    if (addr >= 0x8000 && !prgRom.empty()) {
        return;
    }

    // Otherwise, permit writing to the RAM image for tests
    ram.Data[addr] = value;
}

bool Bus::LoadPRGFromFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open PRG file: " << filename << std::endl;
        return false;
    }

    // Peek header for iNES signature
    char header[16];
    file.read(header, 16);
    if (!file) return false;

    // Reset file to beginning for raw loads
    file.clear();
    file.seekg(0, std::ios::beg);

    prgRom.clear();

    if (std::string(header, header+4) == "NES\x1A") {
        // iNES format
        uint8_t prgBanks = static_cast<uint8_t>(header[4]);
        uint8_t chrBanks = static_cast<uint8_t>(header[5]);
        uint8_t flags6 = static_cast<uint8_t>(header[6]);
        bool hasTrainer = (flags6 & 0x04) != 0;

        // Skip header
        file.seekg(16, std::ios::beg);
        if (hasTrainer) file.seekg(512, std::ios::cur);

        // Load PRG (16KB banks)
        size_t prgSize = size_t(prgBanks) * 16384; // 16KB banks
        prgRom.resize(prgSize);
        file.read(reinterpret_cast<char*>(prgRom.data()), prgSize);
        std::cout << "Loaded iNES PRG ROM: " << prgSize << " bytes (" << int(prgBanks) << " x 16KB banks)\n";

        // Mirroring: bit0 == 1 => vertical, 0 => horizontal
        mirrorVertical = (flags6 & 0x01) != 0;
        std::cout << "Mirror: " << (mirrorVertical ? "vertical" : "horizontal") << "\n";

    
        // Load CHR (8KB banks)
        size_t chrSize = size_t(chrBanks) * 8192; // 8KB banks
        if (chrSize > 0) {
            chrRom.resize(chrSize);
            file.read(reinterpret_cast<char*>(chrRom.data()), chrSize);
            std::cout << "Loaded iNES CHR ROM: " << chrSize << " bytes (" << int(chrBanks) << " x 8KB banks)\n";
        } else {
            // No CHR ROM: provide CHR RAM (8KB) initialized to zero
            chrRom.resize(8192);
            std::fill(chrRom.begin(), chrRom.end(), 0);
            std::cout << "No CHR ROM present in iNES file: allocated 8KB CHR RAM.\n";
        }

        // Create mapper if one is supported
        uint8_t flags7 = static_cast<uint8_t>(header[7]);
        uint8_t mapper = ((flags7 & 0xF0) | (flags6 >> 4));
        if (mapper != 0) {
            Mapper* m = CreateMapperFor(this, mapper, prgRom.size(), chrRom.size());
            if (m) {
                AttachMapper(m);
                std::cout << "Attached mapper " << int(mapper) << " implementation.\n";
            } else {
                std::cerr << "No implementation for mapper " << int(mapper) << ". Running with naive mapping may fail.\n";
            }
        }

        return true;
    }

    // Otherwise treat as raw PRG binary: read all bytes
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    prgRom.resize(size);
    file.read(reinterpret_cast<char*>(prgRom.data()), size);

    std::cout << "Loaded raw PRG file: " << size << " bytes\n";
    return true;
}

