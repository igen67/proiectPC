#pragma once
#include <cstdint>
#include <vector>
#include <string>

class Bus;

class Mapper {
public:
    Bus* bus = nullptr;
    virtual ~Mapper() {}
    virtual uint8_t CPURead(uint16_t addr) = 0;
    virtual void CPUWrite(uint16_t addr, uint8_t value) = 0;
    virtual uint8_t CHRRead(uint16_t addr) = 0;
    // Called when the PPU accesses an address in $0000-$1FFF; used for MMC3 A12 detection
    virtual void OnPPUAddr(uint16_t addr, uint32_t cycles) {}
    // Debug helper: return a concise status string for mapper internals
    virtual std::string DebugString() const { return std::string(); }
};

// Factory helper
Mapper* CreateMapperFor(Bus* bus, int mapperNumber, size_t prgSize, size_t chrSize);
