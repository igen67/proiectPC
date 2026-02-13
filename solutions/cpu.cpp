#include "headers/cpu.h"
#include "headers/ppu.h"
#include "headers/mapper.h"
#include "headers/debugger.h"



Debugger debugger;
// 6502 proccesor emulation,
void PrintTrace(const CPU::CPUTrace& t) {
    std::ofstream file("nesTests/log.txt");

if (!file.is_open()) {
    // handle error
    return;
}
    file<<
        "%04X  %02X %02X %02X  "
        "A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%llu\n",
        t.pc,
        t.opcode, t.op1, t.op2,
        t.A, t.X, t.Y, t.P, int(t.SP),
        t.cycles;

}

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
int abc =0;
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
    SP = 0xFD;
    P = FLAG_U;
    A = X = Y = 0;
    SetFlag(FLAG_I, true);

} 

Word CPU::PullWord(u32& Cycles, Bus& bus) {
    SP++;
    Byte low = bus.read(0x0100 | SP);

    SP++;
    Byte high = bus.read(0x0100 | SP);

    Cycles += 2;
    return (high << 8) | low;
}


void CPU::ADCSetStatus(Byte Value) {
    u32 Result = A + Value + (GetFlag(FLAG_C) ? 1 : 0);
    SetFlag(FLAG_C, Result > 0xFF);
    SetFlag(FLAG_Z, ((Result & 0xFF) == 0));
    SetFlag(FLAG_N, ((Result & 0x80) != 0));
    SetFlag(FLAG_V, (~(A ^ Value) & (A ^ Result) & 0x80) != 0);
    A = Result & 0xFF;
}
void CPU::SBCSetStatus(Byte Value)
{
    // Invert the operand to use ADC logic A - M - (1-C) -> A + ~M + C
    Value = ~Value;
    u32 Result = A + Value + (GetFlag(FLAG_C) ? 1 : 0);
    SetFlag(FLAG_C, Result > 0xFF);
    SetFlag(FLAG_Z, ((Result & 0xFF) == 0));
    SetFlag(FLAG_N, ((Result & 0x80) != 0));
    SetFlag(FLAG_V, ((A ^ Result) & (~A ^ Value) & 0x80) != 0);
    A = Result & 0xFF;
}

bool g_verboseCpu = false;

// Loop detector / hotspot diagnostics: disabled by default to avoid automatic yielding
bool g_loopDetect = false;
uint16_t g_loopLastPage = 0;
uint32_t g_loopStreak = 0;
uint32_t g_loopReportThreshold = 1000; // report every N iterations
uint32_t g_loopSleepThreshold = 50000; // start yielding when extremely hot

void CPU::ExecuteBranch(u32& Cycles, Bus& bus, bool Condition) {
    Byte Offset = FetchByte(Cycles, bus);

    if (Condition) {
        Word oldPC = PC;
        PC = PC + static_cast<int8_t>(Offset);
        Cycles += 1;
        if ((oldPC & 0xFF00) != (PC & 0xFF00)) {
            Cycles += 1;
        }
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
    SetZN(A);
    
}
void CPU::LDXSetStatus() {
    SetZN(X);
}
void CPU::LDYSetStatus() {
    SetZN(Y);
}

void CPU::AndSetStatus() {
    LDASetStatus();
}
void CPU::ASL(Bus& bus, Byte& Value)
{
    Byte TempVal = Value;
    Value=Value << 1;
    SetFlag(FLAG_Z, Value == 0);
    SetFlag(FLAG_C, (TempVal & 0x80) != 0);
    SetFlag(FLAG_N, (Value & 0x80) != 0);

} 
void CPU::LSR(Bus& bus, Byte& Value)
{
    Byte TempVal = Value;
    Value=Value >> 1;
    SetFlag(FLAG_Z, Value == 0);
    SetFlag(FLAG_C, (TempVal & 0x01) != 0);
    SetFlag(FLAG_N, (Value & 0x80) != 0);
} 
    void CPU::ROR(Bus& bus, Byte& Value)
    {
        bool old_carry = GetFlag(FLAG_C);
        SetFlag(FLAG_C, (Value & 0x01) != 0);
        Value = (Value >> 1) | (old_carry ? 0x80 : 0);
        SetZN(Value);
    }
    void CPU::ROL(Bus& bus, Byte& Value)
    {
        bool old_carry = GetFlag(FLAG_C);
        SetFlag(FLAG_C, (Value & 0x80) != 0);
        Value = (Value << 1) | (old_carry ? 0x01 : 0);
        SetZN(Value);
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
    Word CPU::indirectAddrModeY(u32& Cycles, Bus& bus, bool addPageCrossCycle)
    {
        Byte zp = FetchByte(Cycles, bus);
        Byte lo = bus.read(zp);
        Byte hi = bus.read((zp + 1) & 0xFF);
        Word base = lo | (hi << 8);
        Word addr = base + Y;
        bool pageCrossed = ((base & 0xFF00) != (addr & 0xFF00));
        Cycles += 3;
        if (addPageCrossCycle && pageCrossed) {
            Cycles += 1;
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
    if (Interrupt && !GetFlag(FLAG_I))  
    { 
    
        bus.write(0x0100 | SP, (PC >> 8) & 0xFF);  
        SP--;
        bus.write(0x0100 | SP, PC & 0xFF);  
        SP--;

        // Save status register to stack
        Byte status = GetStatus(false);

        bus.write(0x0100 | SP, status);
        SP--;
        SetFlag(FLAG_I, true);
        Word Address = bus.read(0xFFFE) | (bus.read(0xFFFF) << 8);
        PC = Address;
        Cycles += 7; // IRQ handling takes 7 cycles
    }
}


void CPU::HandleNMI(u32& Cycles, Bus& bus) {

    bus.write(0x0100 | SP, (PC >> 8) & 0xFF);
    SP--;

    bus.write(0x0100 | SP, PC & 0xFF);
    SP--;
    Byte status = GetStatus(false);
    status |= 0x20; 


    bus.write(0x0100 | SP, status);
    SP--;


    SetFlag(FLAG_I, true);

    Word Address = bus.read(0xFFFA) | (bus.read(0xFFFB) << 8);
    PC = Address;

    
    Cycles += 7;
}




/*void CPU::Execute(u32& Cycles, Bus& bus) {
    static int pcLogCount = 0;
        u32 before = Cycles;
        // Handle pending OAM DMA per-byte transfer
        while (bus.oamDmaActive) {
            if (bus.oamDmaDummy) {
                // initial dummy cycle before bytes are transferred
                Cycles += 1;
                bus.oamDmaDummy = false;
            } else if (bus.oamDmaIndex < 256) {
                // transfer one byte per CPU cycle: read from CPU memory, write to PPU OAM
                uint16_t src = (uint16_t(bus.oamDmaPage) << 8) | (bus.oamDmaIndex & 0xFF);
                uint8_t val = bus.read(src);
                if (bus.ppu) bus.ppu->WriteOAMByte(bus.oamDmaIndex, val);
                bus.oamDmaIndex++;
                Cycles += 1;
            }
            if (bus.oamDmaIndex >= 256) {
                // DMA finished
                bus.oamDmaActive = false;
                bus.oamDmaIndex = 0;
                bus.oamDmaDummy = true;
                // OAMADDR reset typical behavior
                if (bus.ppu) bus.ppu->WriteOAMByte(0, bus.ppu->GetOAM()[0]);
                break; // resume normal instruction fetch
            }
        }


if (bus.nmiLine) {
    bus.nmiLine = false;

    u32 beforeNmi = Cycles;

    HandleNMI(Cycles, bus);

    u32 delta = Cycles - beforeNmi;
    if (bus.ppu) bus.ppu->StepCycles(delta * 3);

   // return;
} 
        else if (bus.cpu && bus.cpu->Interrupt && !GetFlag(FLAG_I)) {
            IRQ_Handler(Cycles, bus, true);
            if (bus.cpu) bus.cpu->Interrupt = false;
        }
        else if (bus.irqEnable && !GetFlag(FLAG_I)) {
            // Legacy IRQ line (set by mapper writes $E000/$E001)
            IRQ_Handler(Cycles, bus, true);
            bus.irqEnable = false;
        }

        else{
        if (pcLogCount < 500) {
            uint8_t opcode = bus.read(PC);
            std::cout << "[PC-LOG] PC=0x" << std::hex << int(PC)
                      << " OPCODE=0x" << int(opcode) << std::dec << "\n";
            pcLogCount++;
        }

        int Instruction = FetchByte(Cycles, bus);

        if (traceInstructionsRemaining > 0) {
            uint16_t instrAddr = static_cast<uint16_t>(PC - 1);
            traceInstructionsRemaining--;
        }
        
//PrintTrace(trace);

        InvokeInstruction(Instruction, Cycles, bus);

    
    }

        // Advance PPU based on cycles used by this instruction
        u32 delta = Cycles - before;
       if (bus.ppu) bus.ppu->StepCycles(delta * 3);
    }*/

    void CPU::Execute(u32& Cycles, Bus& bus) {
        u32 before = Cycles;

        if (bus.oamDmaActive) {
            // OAM DMA stalls the CPU; no instruction executes.
            while (bus.oamDmaActive) {
                if (bus.oamDmaDummy) {
                    Cycles += 1;
                    bus.oamDmaDummy = false;
                } else if (bus.oamDmaIndex < 256) {
                    uint16_t src = (uint16_t(bus.oamDmaPage) << 8) | (bus.oamDmaIndex & 0xFF);
                    uint8_t val = bus.read(src);
                    if (bus.ppu) bus.ppu->WriteOAMByte(bus.oamDmaIndex, val);
                    bus.oamDmaIndex++;
                    Cycles += 1;
                }
                if (bus.oamDmaIndex >= 256) {
                    bus.oamDmaActive = false;
                    bus.oamDmaIndex = 0;
                    bus.oamDmaDummy = true;
                    break;
                }
            }
            u32 delta = Cycles - before;
            if (bus.ppu) bus.ppu->StepCycles(delta * 3);
            return;
        }

        if (bus.nmiLine) {
            bus.nmiLine = false;
            HandleNMI(Cycles, bus);
            u32 delta = Cycles - before;
            if (bus.ppu) bus.ppu->StepCycles(delta * 3);
            return;
        }

        if (bus.cpu && bus.cpu->Interrupt && !GetFlag(FLAG_I)) {
            IRQ_Handler(Cycles, bus, true);
            bus.cpu->Interrupt = false;
            u32 delta = Cycles - before;
            if (bus.ppu) bus.ppu->StepCycles(delta * 3);
            return;
        }

        if (bus.irqEnable && !GetFlag(FLAG_I)) {
            IRQ_Handler(Cycles, bus, true);
            bus.irqEnable = false;
            u32 delta = Cycles - before;
            if (bus.ppu) bus.ppu->StepCycles(delta * 3);
            return;
        }

        int opcode = FetchByte(Cycles, bus);
        InvokeInstruction(opcode, Cycles, bus);

        u32 delta = Cycles - before;
        if (bus.ppu) bus.ppu->StepCycles(delta * 3);
    }









