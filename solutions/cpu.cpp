#include "headers/cpu.h"
#include "headers/ppu.h"
#include "headers/mapper.h"




// 6502 proccesor emulation,

void PrintStartupDebug(Bus& bus) {
    // Print values at $FFFC/$FFFD (reset vector)
    uint8_t resetLow = bus.read(0xFFFC);
    uint8_t resetHigh = bus.read(0xFFFD);
    uint16_t resetVector = resetLow | (resetHigh << 8);
    std::cout << "[EMU-LOG] Reset vector ($FFFC/$FFFD): 0x" << std::hex << int(resetVector) << " (low=0x" << int(resetLow) << ", high=0x" << int(resetHigh) << ")" << std::dec << std::endl;
    // If Mapper4, print PRG bank mapping
    if (bus.mapper && bus.mapper->DebugString().find("MMC3") != std::string::npos) {
        std::cout << "[EMU-LOG] Mapper4 (MMC3) PRG bank mapping: " << bus.mapper->DebugString() << std::endl;
    }
    // Print bytes at reset vector
    std::cout << "[EMU-LOG] Bytes at reset vector (0x" << std::hex << int(resetVector) << "): ";
    for (int i = 0; i < 16; ++i) {
        std::cout << std::hex << int(bus.read(resetVector + i)) << " ";
    }
    std::cout << std::dec << std::endl;
}
CPU::InstructionHandler CPU::instructionTable[256] = { nullptr };
void CPU::printReg(char reg) {
    if ((unsigned char)reg == 0xFF) {
        std::cout << "Register is NULL (uninitialized)" << std::endl;
    }
    else if(reg == 'A'){
        std::cout << "Register: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(A) << std::dec << std::endl;
    }
    else if(reg == 'X'){
        std::cout << "Register: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(X) << std::dec << std::endl;
    }
    else if(reg == 'Y'){
        std::cout << "Register: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(Y) << std::dec << std::endl;
    }
}

void CPU::modifySP()
{
    if (SP < 0x0100) {
        SP = 0x01FF;
    }
    else if (SP > 0x01FF) {
        SP = 0x0100;
    }
}

Byte CPU::FetchByte(u32& Cycles, Bus& bus) {
    Byte Data = bus.read(PC);
    // debug: printf("Fetching byte: 0x%X at PC: 0x%X\n", Data, PC);
    PC++;
    Cycles++;
    return Data;
}



Word CPU::FetchWord(u32& Cycles, Bus& bus) {
    Word Data = bus.read(PC);
    // debug: std::cout << "FetchWord: Low byte at PC=" << std::hex << PC << " is " << (unsigned)Data << std::endl;
    PC++;
    Data |= (bus.read(PC) << 8);
    // debug: high byte fetch (omitted)
    PC++;
    Cycles += 2;
    return Data;
}



void CPU::Reset(Bus& bus) {
    PC = bus.read(0xFFFC) | (bus.read(0xFFFD) << 8);
    printf("Memory[0xFFFC]: 0x%X\n", bus.read(0xFFFC));
    printf("Memory[0xFFFD]: 0x%X\n", bus.read(0xFFFD));

    printf("Reset Vector: 0x%X\n", PC);
    // Use full 16-bit stack pointer (page 1 range 0x0100-0x01FF)
    SP = 0x01FD;
    C = Z = D = B = V = N = 0;
    A = X = Y = 0;
    I = 1;

} 

Word CPU::PullWord(u32& Cycles, Bus& bus) {
    Byte LowByte = bus.read(SP);
    SP++;
    modifySP();
    Byte HighByte = bus.read(SP);
    SP++;
    modifySP();
    Cycles += 2;

    return static_cast<Word>(HighByte) << 8 | LowByte;
}


void CPU::ADCSetStatus(Byte Value) {
    u32 Result = A + Value + C;
    C = (Result > 0xFF);
    Z = ((Result & 0xFF) == 0);
    N = ((Result & 0x80) != 0);
    V = (~(A ^ Value) & (A ^ Result) & 0x80) != 0;
    A = Result & 0xFF;
}
void CPU::SBCSetStatus(Byte Value)
{
    u32 Result = A - Value -(1 - C);
    C = (Result > 0xFF);
    Z = ((Result & 0xFF) == 0);
    N = ((Result & 0x80) != 0);
    V = (~(A ^ Value) & (A ^ Result) & 0x80) != 0;
    A = Result & 0xFF;
}

bool g_verboseCpu = false;

// Loop detector / hotspot diagnostics: enabled during debugging to detect tight busy-wait loops
bool g_loopDetect = true;
uint16_t g_loopLastPage = 0;
uint32_t g_loopStreak = 0;
uint32_t g_loopReportThreshold = 1000; // report every N iterations
uint32_t g_loopSleepThreshold = 50000; // start yielding when extremely hot

void CPU::ExecuteBranch(u32& Cycles, Bus& bus, bool Condition) {
    Byte Offset = FetchByte(Cycles, bus);

    if (Condition) {
        PC = PC + static_cast<int8_t>(Offset);
        if (g_verboseCpu) {
            std::cout << "Branch Condition: "<<Condition
                << ", New PC: " << PC << std::endl;
        }
    } else {
        if (g_verboseCpu) {
            std::cout << "Branch not taken" << std::endl;
        }
    }
} 

void CPU::LDASetStatus() {
    Z = (A == 0);
    N = (A & 0b10000000) > 0;
    
}
void CPU::LDXSetStatus() {
    Z = (X == 0);
    N = (X & 0b10000000) > 0;
}
void CPU::LDYSetStatus() {
    Z = (Y == 0);
    N = (Y & 0b10000000) > 0;
}

void CPU::AndSetStatus() {
    LDASetStatus();
}
void CPU::ASL(Bus& bus, Byte& Value)
{
    Byte TempVal = Value;
    Value=Value << 1;
    if(Value == 0)
        {
            Z = 1;
        }
    if((TempVal & 0b10000000) == 1)
        {
            C = 1;
        }
    if((Value & 0b10000000) == 1)
        {
            N = 1;
        }

} 
void CPU::LSR(Bus& bus, Byte& Value)
{
    Byte TempVal = Value;
    Value=Value >> 1;
    if(Value == 0)
        {
            Z = 1;
        }
    if((TempVal & 0b00000001) == 1)
        {
            C = 1;
        }
    if((Value & 0b10000000) == 1)
        {
            N = 1;
        }
} 
    void CPU::ROR(Bus& bus, Byte& Value)
    {
        C = Value & 0b00000001;
        Z = Value & 0;
        Value = (Value>>1) | (Value<<(7));
        N = Value & 0b10000000;
    }
    void CPU::ROL(Bus& bus, Byte& Value)
    {
        C = Value & 0b10000000;
        Z = Value & 0;
        Value = (Value<<1) | (Value>>(7));
        N = Value & 0b10000000;
    }



    Word CPU::indirectAddrModeX(u32& Cycles, Bus& bus)
    {
        Byte zp = FetchByte(Cycles, bus);
        zp = zp + X;
        Byte lo = bus.read(zp & 0xFF);
        Byte hi = bus.read((zp + 1) & 0xFF);
        Word FinalAddr = lo | (hi << 8);
        Cycles += 4;
        return FinalAddr;
    }
    Word CPU::indirectAddrModeY(u32& Cycles, Bus& bus)
    {
        Byte zp = FetchByte(Cycles, bus);
        Byte lo = bus.read(zp);
        Byte hi = bus.read((zp + 1) & 0xFF);
        Word base = lo | (hi << 8);
        Word addr = base + Y;
        if ((base & 0xFF00) != (addr & 0xFF00)) {
            Cycles += 4;
        } else {
            Cycles += 3;
        }
        return addr;
    }
void CPU::startProg(Bus& bus, u32 cycles)
{
    Reset(bus);
    Execute(cycles, bus);
} 
CPU::InstructionHandler CPU::GetInstructionHandler(Byte opcode) {

    if (opcode < 0xFF) {
        return instructionTable[opcode];
    }
    else {

        std::cout << "Invalid opcode: " << (opcode) << std::endl;
        return nullptr;
    }
}

void CPU::InvokeInstruction(Byte opcode, u32& Cycles, Bus& bus) {
    CPU::InstructionHandler handler = CPU::GetInstructionHandler(opcode);
    if (handler != nullptr) {
        handler(*this, Cycles, bus);
    }
    else {
        static bool warnedOnce = false;
        if (!warnedOnce) {
            std::cout << "Warning: no handler for opcode 0x" << std::hex << std::uppercase << static_cast<int>(opcode) << std::dec << " - treating as NOP" << std::endl;
            warnedOnce = true;
        }
        // Treat missing/illegal opcode as a single-byte NOP (best-effort to continue execution)
        // Note: This hides real errors; once things are stable we may want to fail instead.
        return;
    }
}
void CPU::IRQ_Handler(u32& Cycles, Bus& bus, bool Interrupt)
{
    if (Interrupt && I == 0)  // Check if interrupt is requested and I flag is clear
    {
        // Log and acknowledge the IRQ
        std::cout << "[IRQ] Taking IRQ at PC=0x" << std::hex << PC << std::dec << " SP=0x" << std::hex << SP << std::dec << std::endl;
        if (bus.cpu) bus.cpu->Interrupt = false; // acknowledge the line so it won't retrigger immediately

        // Save PC to stack (low byte first) - pre-decrement to match PHA/PLA convention
        SP--;
        modifySP();
        bus.write(SP, PC & 0xFF);  // Low byte
        std::cout << "[IRQ] SP after pushing low PC: 0x" << std::hex << SP << std::dec << std::endl;

        SP--;
        modifySP();
        bus.write(SP, (PC >> 8));  // High byte
        std::cout << "[IRQ] SP after pushing high PC: 0x" << std::hex << SP << std::dec << std::endl;

        // Save status register to stack
        Byte status = 0;
        if (C == 1) status |= 0b00000001;
        if (Z == 1) status |= 0b00000010;
        if (I == 1) status |= 0b00000100;
        if (D == 1) status |= 0b00001000;
                    status |= 0b00010000;  // Set only for BRK
                    status |= 0b00100000;
        if (V == 1) status |= 0b01000000;
        if (N == 1) status |= 0b10000000;

        SP--;
        modifySP();
        bus.write(SP, status);
        std::cout << "[IRQ] SP after pushing status: 0x" << std::hex << SP << std::dec << std::endl;


        // Set I flag
        I = 1;

        // Fetch IRQ vector and set PC
        Word Address = bus.read(0xFFFE) | (bus.read(0xFFFF) << 8);
        std::cout << "[IRQ] Vector -> 0x" << std::hex << Address << std::dec << std::endl;
        PC = Address;
    }
}

// Non-Maskable Interrupt handler (NMI)
void CPU::HandleNMI(u32& Cycles, Bus& bus) {
    // Log NMI for debugging
    std::cout << "[NMI] Taking NMI at PC=0x" << std::hex << PC << std::dec << " SP=0x" << std::hex << SP << std::dec << std::endl;

    // Save PC to stack (low byte first) - pre-decrement to match PHA/PLA convention
    SP--;
    modifySP();
    bus.write(SP, PC & 0xFF);
    std::cout << "[NMI] SP after pushing low PC: 0x" << std::hex << SP << std::dec << std::endl;

    SP--;
    modifySP();
    bus.write(SP, (PC >> 8));
    std::cout << "[NMI] SP after pushing high PC: 0x" << std::hex << SP << std::dec << std::endl;

    // Save status register to stack (B flag is cleared for NMI)
    Byte status = 0;
    if (C == 1) status |= 0b00000001;
    if (Z == 1) status |= 0b00000010;
    if (I == 1) status |= 0b00000100;
    if (D == 1) status |= 0b00001000;
    // B flag should be 0 for NMI
    if (V == 1) status |= 0b01000000;
    if (N == 1) status |= 0b10000000;

    SP--;
    modifySP();
    bus.write(SP, status);

    // Set I flag to disable further IRQs during NMI handling
    I = 1;

    // Fetch NMI vector and set PC
    Word Address = bus.read(0xFFFA) | (bus.read(0xFFFB) << 8);
    std::cout << "CPU: NMI vector read -> 0x" << std::hex << Address << std::dec << std::endl;
    PC = Address;
    // Dump a few bytes at NMI address for inspection
    std::cout << "CPU: bytes @NMI: ";
    for (int i = 0; i < 16; ++i) {
        uint16_t a = static_cast<uint16_t>(PC + i);
        std::cout << std::hex << int(bus.read(a)) << " ";
    }
    std::cout << std::dec << std::endl;

    // Start a short post-NMI instruction trace to help debug initialization behavior
    traceInstructionsRemaining = 256; // trace next 256 instructions
    std::cout << "TRACE: Starting NMI instruction trace (" << traceInstructionsRemaining << " instrs) at 0x" << std::hex << PC << std::dec << std::endl;
    // Dump a small memory window at the NMI handler for quick inspection
    std::cout << "TRACE: Memory @NMI: ";
    for (int i = 0; i < 64; ++i) {
        uint16_t a = static_cast<uint16_t>(PC + i);
        std::cout << std::hex << int(bus.read(a)) << " ";
    }
    std::cout << std::dec << std::endl;
}



// Execute a single instruction (used by GUI to step/run)
void CPU::Step(u32& Cycles, Bus& bus) {
    u32 before = Cycles;
    Byte opcode = FetchByte(Cycles, bus);
       static bool printedStartup = false;
    if (!printedStartup) {
        PrintStartupDebug(bus);
        printedStartup = true;
    }

    // Optional instruction tracing (enabled briefly after NMI)
    if (traceInstructionsRemaining > 0) {
        if (traceInstructionsRemaining <= 16) {
            uint16_t instrAddr = static_cast<uint16_t>(PC - 1);
            std::cout << "TRACE: PC=0x" << std::hex << instrAddr << " opcode=0x" << int(opcode) << " bytes:";
            for (int i = 0; i < 6; ++i) std::cout << " " << std::hex << int(bus.read(instrAddr + i));
            std::cout << std::dec << std::endl;
        }
        traceInstructionsRemaining--;
        if (traceInstructionsRemaining == 0) {
            if (bus.ppu) {
                const uint8_t* pal = bus.ppu->GetPaletteRam();
                const uint8_t* oam = bus.ppu->GetOAM();
                std::cout << "TRACE: PPU Palette:";
                for (int i = 0; i < 32; ++i) std::cout << " " << std::hex << int(pal[i]);
                std::cout << std::dec << std::endl;
                std::cout << "TRACE: PPU OAM (first 64 bytes):";
                for (int i = 0; i < 64; ++i) std::cout << " " << std::hex << int(oam[i]);
                std::cout << std::dec << std::endl;
            }
        }
    }

    // Loop detection & hotspot diagnostics for suspicious PC ranges (e.g., $FFF0-$FFFF, $F400-$F4FF)
    {
        uint16_t instrAddr = static_cast<uint16_t>(PC - 1);
        if (g_loopDetect) {
            uint16_t page = instrAddr & 0xFFF0;
            if (page == g_loopLastPage) {
                ++g_loopStreak;
            } else {
                g_loopLastPage = page;
                g_loopStreak = 1;
            }

            // If we're in known suspicious ranges report occasionally and yield when extremely hot
            if (instrAddr >= 0xFFF0 || (instrAddr >= 0xF400 && instrAddr < 0xF500)) {
                if ((g_loopStreak % g_loopReportThreshold) == 0) {
                    std::cout << "[LOOP] Hotspot page=0x" << std::hex << page << " PC=0x" << instrAddr << std::dec
                              << " count=" << g_loopStreak << " I=" << int(I)
                              << " Interrupt=" << (Interrupt?1:0) << " NMIRequested=" << (NMIRequested?1:0);
                    if (bus.mapper) std::cout << " Mapper(" << bus.mapper->DebugString() << ")";
                    std::cout << std::endl;
                    std::cout << "[LOOP] bytes@0x" << std::hex << page << ":";
                    for (int i = 0; i < 16; ++i) std::cout << " " << std::hex << int(bus.read(page + i));
                    std::cout << std::dec << std::endl;
                }
            }
        }
    }

    InvokeInstruction(opcode, Cycles, bus);
    // Handle pending NMI (non-maskable) before maskable IRQ
    if (NMIRequested) {
        NMIRequested = false;
        HandleNMI(Cycles, bus);
    }
    IRQ_Handler(Cycles, bus, Interrupt);
    u32 delta = Cycles - before;
    if (bus.ppu) bus.ppu->StepCycles(delta * 3);
    
}


void CPU::Execute(u32& Cycles, Bus& bus) {
    while (true) {
        u32 before = Cycles;
        int Instruction = FetchByte(Cycles, bus);

        if (traceInstructionsRemaining > 0) {
            uint16_t instrAddr = static_cast<uint16_t>(PC - 1);
            std::cout << "TRACE: PC=0x" << std::hex << instrAddr << " opcode=0x" << int(Instruction) << " bytes:";
            for (int i = 0; i < 6; ++i) std::cout << " " << std::hex << int(bus.read(instrAddr + i));
            std::cout << std::dec << std::endl;
            traceInstructionsRemaining--;
            if (traceInstructionsRemaining == 0) {
                if (bus.ppu) {
                    const uint8_t* pal = bus.ppu->GetPaletteRam();
                    const uint8_t* oam = bus.ppu->GetOAM();
                    const uint8_t* vram = bus.ppu->GetVRAM();
                    std::cout << "TRACE: PPU Palette:";
                    for (int i = 0; i < 32; ++i) std::cout << " " << std::hex << int(pal[i]);
                    std::cout << std::dec << std::endl;
                    std::cout << "TRACE: PPU OAM (first 64 bytes):";
                    for (int i = 0; i < 64; ++i) std::cout << " " << std::hex << int(oam[i]);
                    std::cout << std::dec << std::endl;
                    std::cout << "TRACE: PPU VRAM (first 64 bytes):";
                    for (int i = 0; i < 64; ++i) std::cout << " " << std::hex << int(vram[i]);
                    std::cout << std::dec << std::endl;
                    std::cout << "TRACE: PPU VRAM @0x400 (nametable 1) 64 bytes:";
                    for (int i = 0x400; i < 0x440; ++i) std::cout << " " << std::hex << int(vram[i]);
                    std::cout << std::dec << std::endl;
                    std::cout << "TRACE: PPU VRAM @0x800 (nametable 2) 64 bytes:";
                    for (int i = 0x800; i < 0x840; ++i) std::cout << " " << std::hex << int(vram[i]);
                    std::cout << std::dec << std::endl;
                }
            }
        }

        InvokeInstruction(Instruction, Cycles, bus);
        // Tight-loop detection (same logic as Step) to catch busy-wait loops during Execute()
        if (g_loopDetect) {
            uint16_t instrAddr = static_cast<uint16_t>(PC - 1);
            uint16_t page = instrAddr & 0xFFF0;
            if (page == g_loopLastPage) {
                ++g_loopStreak;
            } else {
                g_loopLastPage = page;
                g_loopStreak = 1;
            }
            if (instrAddr >= 0xFFF0 || (instrAddr >= 0xF400 && instrAddr < 0xF500)) {
                if ((g_loopStreak % g_loopReportThreshold) == 0) {
                    std::cout << "[LOOP] Hotspot page=0x" << std::hex << page << " PC=0x" << instrAddr << std::dec
                              << " count=" << g_loopStreak << " I=" << int(I)
                              << " Interrupt=" << (Interrupt?1:0) << " NMIRequested=" << (NMIRequested?1:0);
                    if (bus.mapper) std::cout << " Mapper(" << bus.mapper->DebugString() << ")";
                    std::cout << std::endl;
                }
                if (g_loopStreak > g_loopSleepThreshold) {
                }
            }
        }
        if (std::cin.get() == ' ') {
            continue;
        }
        IRQ_Handler(Cycles, bus, Interrupt);
        // Advance PPU based on cycles used by this instruction
        u32 delta = Cycles - before;
        if (bus.ppu) bus.ppu->StepCycles(delta * 3);
        std::this_thread::sleep_for(std::chrono::milliseconds((1 / (40 * (1000000))) * Cycles));
        Cycles = 0;
        std::cerr << "Registrele A, X , Y";
        printReg('A');
        printReg('X');
        printReg('Y');
        std::cerr << std::endl;
    }
}









