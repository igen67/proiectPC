#include "headers/mapper.h"
#include "headers/bus.h"
#include "headers/cpu.h"
#include <cstring>
#include <iostream>
#include <cstdio>

static bool g_mapperVerbose = false; // set true to enable detailed mapper logs
static bool g_mapperIRQLog = true;    // set true to enable IRQ-specific logs

// Minimal MMC3 (Mapper 4) implementation. This supports PRG/CHR bank switching and IRQ scanline counting
// sufficiently for many games (including SMB) in a basic form.
class Mapper4 : public Mapper {
public:
    std::vector<uint8_t> prgRam; // 8KB PRG RAM at $6000
    uint8_t bankRegs[8] = {0};
    uint8_t bankSelect = 0;
    bool prgMode = false; // PRG mode bit (bit 6 of bankSelect)
    bool chrMode = false;  // CHR mode bit (bit 7 of bankSelect)
    // MMC3 IRQ
    uint8_t irqLatch = 0;
    uint8_t irqCounter = 0;
    bool irqReload = false;


    // A12 tracking
    bool prevA12 = false;
    int a12HighCycles = 0; // not cycle-accurate but approximates the detection
    int a12EdgeCount = 0; // count rising edges to limit logging frequency

    size_t prgBankCount = 0;
    size_t chrBankCount = 0;

    Mapper4(Bus* b, size_t prgSize, size_t chrSize) {
        bus = b;
        prgBankCount = prgSize / 0x2000; // 8KB units
        chrBankCount = chrSize / 0x0400; // 1KB units
        prgRam.resize(0x2000);
    }

    uint8_t CPURead(uint16_t addr) override {
        if (addr >= 0x6000 && addr < 0x8000) {
            return prgRam[addr - 0x6000];
        }
        if (addr >= 0x8000) {
            // Map PRG banks in 8KB windows
            return ReadPRG(addr);
        }
        return 0;
    }

    void CPUWrite(uint16_t addr, uint8_t value) override {
        if (addr >= 0x6000 && addr < 0x8000) {
            prgRam[addr - 0x6000] = value;
            return;
        }
        if (addr >= 0x8000 && addr <= 0x9FFF) {
            if ((addr & 1) == 0) {
                // bank select
                bankSelect = value & 0x07;
                prgMode = value & 0x40;
                chrMode = value & 0x80;


                if (g_mapperVerbose) std::cout << "Mapper4: bank select=" << int(bankSelect)
                          << " prgMode=" << prgMode << " chrMode=" << chrMode << std::endl;
            } else {
                // bank data
                if (bankSelect < 8) {
                    bankRegs[bankSelect] = value;
                    if (g_mapperVerbose) std::cout << "Mapper4: bank data reg[" << int(bankSelect) << "] = " << int(value) << std::endl;
                } else {
                    std::cout << "Mapper4: bank data reg[" << int(bankSelect) << "] OUT OF RANGE!" << std::endl;
                }
            }
            return;
        }
        if (addr >= 0xA000 && addr <= 0xBFFF) {
            if ((addr & 1) == 0) {
                // mirroring
                bus->mirrorVertical = (value & 1) != 0;
                mirroring = bus->mirrorVertical ? Mirroring::Vertical : Mirroring::Horizontal;
                if (g_mapperVerbose) std::cout << "Mapper4: mirroring set to " << (bus->mirrorVertical ? "vertical" : "horizontal") << std::endl;
            } else {
                // PRG RAM protect - ignored for now
                if (g_mapperVerbose) std::cout << "Mapper4: PRG RAM protect write ignored" << std::endl;
            }
            return;
        }
        if (addr >= 0xC000 && addr <= 0xDFFF) {
            if ((addr & 1) == 0) {
                irqLatch = value;
                if (g_mapperIRQLog) std::cout << "Mapper4: IRQ latch set to " << int(value) << std::endl;
            } else {
                irqReload = true;
                if (g_mapperIRQLog) std::cout << "Mapper4: IRQ reload triggered" << std::endl;
            }
            return;
        }
        if (addr >= 0xE000) {
            if ((addr & 1) == 0) {
                // disable
                bus->irqEnable = false;
                if (bus && bus->cpu) bus->cpu->Interrupt = false;
                if (g_mapperIRQLog) std::cout << "Mapper4: IRQ disabled" << std::endl;
            } else {
                bus->irqEnable = true;
                if (g_mapperIRQLog) std::cout << "Mapper4: IRQ enabled" << std::endl;
            }
            return;
        }
    }



    uint8_t CHRRead(uint16_t addr) override {
        // Implement CHR mapping according to chrMode and bank registers
        // addr: 0x0000 - 0x1FFF
        uint32_t a = addr;
        uint32_t bankIndex = 0;
        uint32_t offset = 0;
        if (!chrMode) {
            // Mode 0: r0/r1 are 2KB at $0000/$0800, r2..r5 1KB each at $1000..$1FFF
            if (a < 0x0800) {
                uint32_t bank = bankRegs[0] & 0xFE; // FORCE EVEN
                uint32_t abs = bank * 0x400 + offset;
                return bus->chrRom[abs];

            } else if (a < 0x1000) {
                uint32_t bank = bankRegs[1] & 0xFE;
                uint32_t abs = bank * 0x400 + offset;
                return bus->chrRom[abs];
            } else if (a < 0x1400) {
                bankIndex = bankRegs[2] & 0xFF;
                offset = a & 0x3FF;
                return ReadCHRBank(bankIndex, offset);
            } else if (a < 0x1800) {
                bankIndex = bankRegs[3] & 0xFF;
                offset = a & 0x3FF;
                return ReadCHRBank(bankIndex, offset);
            } else if (a < 0x1C00) {
                bankIndex = bankRegs[4] & 0xFF;
                offset = a & 0x3FF;
                return ReadCHRBank(bankIndex, offset);
            } else {
                bankIndex = bankRegs[5] & 0xFF;
                offset = a & 0x3FF;
                return ReadCHRBank(bankIndex, offset);
            }
        } else {
            // Mode 1: r2..r5 are first 4 1KB at $0000..$0FFF and r0/r1 are 2KB at $1000/$1800
            if (a < 0x0400) {
                bankIndex = bankRegs[2] & 0xFF; offset = a & 0x3FF; return ReadCHRBank(bankIndex, offset);
            } else if (a < 0x0800) {
                bankIndex = bankRegs[3] & 0xFF; offset = a & 0x3FF; return ReadCHRBank(bankIndex, offset);
            } else if (a < 0x0C00) {
                bankIndex = bankRegs[4] & 0xFF; offset = a & 0x3FF; return ReadCHRBank(bankIndex, offset);
            } else if (a < 0x1000) {
                bankIndex = bankRegs[5] & 0xFF; offset = a & 0x3FF; return ReadCHRBank(bankIndex, offset);
            } else if (a < 0x1800) {
                bankIndex = bankRegs[0] & 0xFF; offset = a & 0x7FF; return ReadCHRBank(bankIndex, offset);
            } else {
                bankIndex = bankRegs[1] & 0xFF; offset = a & 0x7FF; return ReadCHRBank(bankIndex, offset);
            }
        }
    }

    void CHRWrite(uint16_t addr, uint8_t value) override {
        if (!bus || !bus->chrIsRam) {
            return;
        }
        uint32_t a = addr;
        uint32_t bankIndex = 0;
        uint32_t offset = 0;
        if (!chrMode) {
            if (a < 0x0800) {
                uint32_t bank = bankRegs[0] & 0xFE;
                uint32_t abs = bank * 0x400 + offset;
                if (abs < bus->chrRom.size()) bus->chrRom[abs] = value;
                return;
            } else if (a < 0x1000) {
                uint32_t bank = bankRegs[1] & 0xFE;
                uint32_t abs = bank * 0x400 + offset;
                if (abs < bus->chrRom.size()) bus->chrRom[abs] = value;
                return;
            } else if (a < 0x1400) {
                bankIndex = bankRegs[2] & 0xFF;
                offset = a & 0x3FF;
            } else if (a < 0x1800) {
                bankIndex = bankRegs[3] & 0xFF;
                offset = a & 0x3FF;
            } else if (a < 0x1C00) {
                bankIndex = bankRegs[4] & 0xFF;
                offset = a & 0x3FF;
            } else {
                bankIndex = bankRegs[5] & 0xFF;
                offset = a & 0x3FF;
            }
        } else {
            if (a < 0x0400) {
                bankIndex = bankRegs[2] & 0xFF; offset = a & 0x3FF;
            } else if (a < 0x0800) {
                bankIndex = bankRegs[3] & 0xFF; offset = a & 0x3FF;
            } else if (a < 0x0C00) {
                bankIndex = bankRegs[4] & 0xFF; offset = a & 0x3FF;
            } else if (a < 0x1000) {
                bankIndex = bankRegs[5] & 0xFF; offset = a & 0x3FF;
            } else if (a < 0x1800) {
                bankIndex = bankRegs[0] & 0xFF; offset = a & 0x7FF;
                uint32_t abs = bankIndex * 0x400 + offset;
                if (abs < bus->chrRom.size()) bus->chrRom[abs] = value;
                return;
            } else {
                bankIndex = bankRegs[1] & 0xFF; offset = a & 0x7FF;
                uint32_t abs = bankIndex * 0x400 + offset;
                if (abs < bus->chrRom.size()) bus->chrRom[abs] = value;
                return;
            }
        }
        uint32_t abs = bankIndex * 0x400 + offset;
        if (abs < bus->chrRom.size()) bus->chrRom[abs] = value;
    }

    uint8_t ReadPRG(uint16_t addr) {
        // PRG banks are 8KB units
        uint32_t bank = 0;
        uint32_t bankOffset = 0;
        uint32_t rel = addr - 0x8000;
        uint32_t slot = rel / 0x2000; // 0..3
        uint32_t inner = rel & 0x1FFF;

        uint32_t lastBank = (uint32_t)prgBankCount - 1;
        uint32_t secondLast = (uint32_t)prgBankCount - 2;

        if (!prgMode) {
            switch (slot) {
                case 0: bank = bankRegs[6]; break; // $8000
                case 1: bank = bankRegs[7]; break; // $A000
                case 2: bank = secondLast; break;  // $C000
                case 3: bank = lastBank; break;    // $E000
            }
        } else {
            switch (slot) {
                case 0: bank = secondLast; break;
                case 1: bank = bankRegs[7]; break;
                case 2: bank = bankRegs[6]; break;
                case 3: bank = lastBank; break;
            }
        }

        // Bounds check for PRG bank
        if (bank >= prgBankCount) {
            std::cout << "Mapper4: PRG bank " << bank << " OUT OF RANGE! (max=" << prgBankCount-1 << ")" << std::endl;
            bank = lastBank;
        }

        uint32_t absAddr = bank * 0x2000 + inner;
        if (addr >= 0xFF00) {
            std::cout << "Mapper4: ReadPRG addr=0x" << std::hex << addr << " slot=" << std::dec << slot
                      << " bank=" << bank << " absAddr=0x" << std::hex << absAddr << std::dec << std::endl;
        }
        if (absAddr < bus->prgRom.size()) return bus->prgRom[absAddr];
        return 0xFF;
    }
    uint8_t ReadCHR1K(uint32_t bank, uint32_t offset) {
    uint32_t abs = bank * 0x400 + offset;
    if (abs < bus->chrRom.size()) return bus->chrRom[abs];
    return 0;
}

uint8_t ReadCHR2K(uint32_t bank, uint32_t offset) {
    uint32_t abs = bank * 0x800 + offset;
    if (abs < bus->chrRom.size()) return bus->chrRom[abs];
    return 0;
}


uint8_t ReadCHRBank(uint32_t bankIndex, uint32_t offset) {
    uint32_t abs = bankIndex * 0x400 + offset;
    if (abs < bus->chrRom.size())
        return bus->chrRom[abs];
    return 0;
}


    void OnPPUAddr(uint16_t addr, uint32_t cycles) override {
        // detect rising edge of A12 (bit 12 of addr)
        bool a12 = (addr & 0x1000) != 0;
        if (!prevA12 && a12) {
            // rising edge
            a12EdgeCount++;
            if (irqCounter == 0 || irqReload) {
                irqCounter = irqLatch;
                irqReload = false;
            } else {
                --irqCounter;
            }
            // Log sparingly: focus on IRQ events
            if ((a12EdgeCount & 31) == 0 && g_mapperVerbose) {
                std::cout << "Mapper4: A12 rising edge #" << a12EdgeCount
                          << " irqCounter=" << int(irqCounter)
                          << " irqLatch=" << int(irqLatch)
                          << " irqEnable=" << bus->irqEnable << std::endl;
            }
            if (irqCounter == 0 && bus->irqEnable) {
                // Request IRQ on CPU
                if (bus && bus->cpu) {
                    bus->cpu->Interrupt = true;
                    if (g_mapperIRQLog) std::cout << "Mapper4: IRQ asserted (edgeCount=" << a12EdgeCount << ")" << std::endl;
                }
            }
        }
        prevA12 = a12;
    }

    // Debug helper
    std::string DebugString() const override {
        char buf[256];
        snprintf(buf, sizeof(buf), "irqEnable=%d irqCounter=%d irqLatch=%d prgMode=%d chrMode=%d bank6=%u bank7=%u",
                 bus->irqEnable?1:0, irqCounter, irqLatch, prgMode?1:0, chrMode?1:0, bankRegs[6], bankRegs[7]);
        return std::string(buf);
    }
};
class Mapper0 : public Mapper {

public:
    size_t prgBanks = 0;

    Mapper0(Bus* b, size_t prgSize) {
        bus = b;
        prgBanks = prgSize / 0x4000;
        mirroring = (bus && bus->mirrorVertical) ? Mirroring::Vertical : Mirroring::Horizontal;
    }

    uint8_t CPURead(uint16_t addr) override {
        if (addr >= 0x8000) {
            uint32_t mapped = addr - 0x8000;
            if (prgBanks == 1) {
                mapped &= 0x3FFF; // mirror 16 KB
            }
            return bus->prgRom[mapped];
        }
        return 0;
    }

    void CPUWrite(uint16_t, uint8_t) override {
        // NROM is read-only
    }

    uint8_t CHRRead(uint16_t addr) override {
        return bus->chrRom[addr & 0x1FFF];
    }

    void CHRWrite(uint16_t addr, uint8_t value) override {
        if (!bus || !bus->chrIsRam) {
            return;
        }
        uint32_t idx = addr & 0x1FFF;
        if (idx < bus->chrRom.size()) {
            bus->chrRom[idx] = value;
        }
    }

    Mirroring GetMirroring() const override {
        return mirroring;
    }
};

Mapper* CreateMapperFor(Bus* bus, int mapperNumber, size_t prgSize, size_t chrSize) {
    if (mapperNumber == 0) {
    return new Mapper0(bus, prgSize);
}
if (mapperNumber == 4) {
    return new Mapper4(bus, prgSize, chrSize);
}
return nullptr;

}
