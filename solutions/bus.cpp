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
    chrIsRam = false;
    irqEnable = false;
}



uint8_t Bus::read(uint16_t addr)  {
    // RAM and mirrors
    if (addr <= 0x1FFF) {
        return ram.Data[addr & RAM_MASK];
    }

    // PPU registers mirrored every 8 bytes: $2000-$3FFF
    if (addr >= 0x2000 && addr <= 0x3FFF && ppu) {
        uint16_t reg = (addr - 0x2000) & 0x7;
        return ppu->ReadRegister(reg);
    }

    // APU / I/O registers (0x4000 - 0x4017)
    if (addr >= 0x4000 && addr <= 0x4017) {
        if (addr == 0x4016) {
            return input.Read(0);
        }
        if (addr == 0x4017) {
            //ADD APU FRAME COUNTER
            return input.Read(1);
        }
        // Unimplemented APU / IO registers: return 0
        return 0;
    }

    // Mapper-aware PRG area
    if (mapper) {
        // Mapper handles CPU read mapping for $6000-$FFFF
        if (addr >= 0x6000) return mapper->CPURead(addr);
    }

    // (controller reads are handled via Input at 0x4016/0x4017 above)

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
    return 0; // or open bus
}

uint8_t Bus::ReadCHR(uint16_t addr) const {
    if (mapper) {
        return mapper->CHRRead(addr);
    }
    // If no mapper is present, read directly from loaded CHR ROM/RAM
    if (!chrRom.empty()) {
        if (addr < chrRom.size()) return chrRom[addr];
        // Safety: wrap or clamp if address outside CHR size
        return chrRom[addr % chrRom.size()];
    }
    return 0;
}

void Bus::WriteCHR(uint16_t addr, uint8_t value) {
    if (mapper) {
        mapper->CHRWrite(addr, value);
        return;
    }
    if (chrIsRam && addr < chrRom.size()) {
        chrRom[addr] = value;
    }
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
        Word reg = addr & 0x7;
        ppu->WriteRegister(reg, value);
        return;
    }

    // OAM DMA write (0x4014): initialize per-byte DMA state for cycle-accurate transfer
    if (addr == 0x4014 && ppu) {
        this->oamDmaActive = true;
        this->oamDmaPage = value;
        this->oamDmaIndex = 0;
        this->oamDmaDummy = true; // initial dummy cycle
        return;
    }

    // Controller strobe write (0x4016): forward to Input
    if (addr == 0x4016) {
        this->input.WriteStrobe(value);
        return;
    }

    // Mapper-aware PRG area handling
    if (mapper) {
        if (addr >= 0x6000) {
            mapper->CPUWrite(addr, value);
            return;
        }
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

        Mirroring cartMirroring = Mirroring::Horizontal;
        if (flags6 & 0x08) {
            cartMirroring = Mirroring::FourScreen;
        } else if (flags6 & 0x01) {
            cartMirroring = Mirroring::Vertical;
        }
        mirrorVertical = (cartMirroring == Mirroring::Vertical);
        if (cartMirroring == Mirroring::FourScreen) {
            std::cout << "Mirror: four-screen\n";
        } else {
            std::cout << "Mirror: " << (mirrorVertical ? "vertical" : "horizontal") << "\n";
        }

    
        // Load CHR (8KB banks)
        size_t chrSize = size_t(chrBanks) * 8192; // 8KB banks
        if (chrSize > 0) {
            chrRom.resize(chrSize);
            file.read(reinterpret_cast<char*>(chrRom.data()), chrSize);
            std::cout << "Loaded iNES CHR ROM: " << chrSize << " bytes (" << int(chrBanks) << " x 8KB banks)\n";
            chrIsRam = false;
            if (!chrRom.empty()) {
                uint32_t sum = 0;
                for (uint8_t b : chrRom) sum += b;
                std::cout << "CHR sum: 0x" << std::hex << sum << std::dec << "\n";
                std::cout << "CHR first 32: ";
                size_t headCount = std::min<size_t>(32, chrRom.size());
                for (size_t i = 0; i < headCount; ++i) {
                    std::cout << std::hex << int(chrRom[i]) << " ";
                }
                std::cout << std::dec << "\n";
                std::cout << "CHR last 32: ";
                size_t tailCount = std::min<size_t>(32, chrRom.size());
                size_t start = chrRom.size() - tailCount;
                for (size_t i = start; i < chrRom.size(); ++i) {
                    std::cout << std::hex << int(chrRom[i]) << " ";
                }
                std::cout << std::dec << "\n";
            }
        } else {
            // No CHR ROM: provide CHR RAM (8KB) initialized to zero
            chrRom.resize(8192);
            std::fill(chrRom.begin(), chrRom.end(), 0);
            std::cout << "No CHR ROM present in iNES file: allocated 8KB CHR RAM.\n";
            chrIsRam = true;
        }

        // Create mapper if one is supported
        uint8_t flags7 = static_cast<uint8_t>(header[7]);
        uint8_t mapper = ((flags7 & 0xF0) | (flags6 >> 4));
        Mapper* m = CreateMapperFor(this, mapper, prgRom.size(), chrRom.size());
        if (m) {
            AttachMapper(m);
            m->mirroring = cartMirroring;
            std::cout << "Attached mapper " << int(mapper) << " implementation.\n";
        } else if (mapper != 0) {
            std::cerr << "No implementation for mapper " << int(mapper) << ". Running with naive mapping may fail.\n";
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
    chrIsRam = false;
    return true;
}



