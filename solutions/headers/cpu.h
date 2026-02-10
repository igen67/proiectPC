#pragma once



#include "instructions.h"
#include "bus.h"

using namespace Instructions;

// Global verbose flag to enable CPU trace prints (default off)
extern bool g_verboseCpu;

struct CPU
{
    Word PC; // program counter
    Byte SP; // stack pointer

    Byte A, X, Y; // regs

    static constexpr uint8_t FLAG_C = 0x01; // carry
    static constexpr uint8_t FLAG_Z = 0x02; // zero
    static constexpr uint8_t FLAG_I = 0x04; // interrupt disable
    static constexpr uint8_t FLAG_D = 0x08; // decimal
    static constexpr uint8_t FLAG_B = 0x10; // break
    static constexpr uint8_t FLAG_U = 0x20; // unused, always 1
    static constexpr uint8_t FLAG_V = 0x40; // overflow
    static constexpr uint8_t FLAG_N = 0x80; // negative

    uint8_t P = FLAG_U; // status register (bit 5 always 1)

    typedef void (*InstructionHandler)(CPU& cpu, u32& Cycles, Bus& bus);
    static InstructionHandler instructionTable[256];

    bool Interrupt = false;


    // NMI request (set by PPU on VBlank)
    bool NMIRequested = false;
    bool prevNmiLine = false;

    // Tracing: when >0, CPU will print executed instructions and bus interactions for debugging
    int traceInstructionsRemaining = 0;
    void HandleNMI(u32& Cycles, Bus& bus);

    void printReg(char reg);
    void modifySP();
    bool GetFlag(uint8_t flag) const { return (P & flag) != 0; }
    void SetFlag(uint8_t flag, bool value) {
        if (value) P |= flag;
        else P &= static_cast<uint8_t>(~flag);
        P |= FLAG_U;
    }
    void SetZN(uint8_t value) {
        SetFlag(FLAG_Z, value == 0);
        SetFlag(FLAG_N, (value & 0x80) != 0);
    }
    uint8_t GetStatus(bool breakFlag) const {
        uint8_t status = static_cast<uint8_t>(P | FLAG_U);
        if (breakFlag) status |= FLAG_B;
        else status &= static_cast<uint8_t>(~FLAG_B);
        return status;
    }
    void SetStatusFromStack(uint8_t status) {
        P = static_cast<uint8_t>((status | FLAG_U) & ~FLAG_B);
    }
    Byte FetchByte(u32& Cycles, Bus& bus);
    Word FetchWord(u32& Cycles, Bus& bus);
    void Reset(Bus& bus);
    Word PullWord(u32& Cycles, Bus& bus);
    void ADCSetStatus(Byte Value);
    void SBCSetStatus(Byte Value);
    void ExecuteBranch(u32& Cycles, Bus& bus, bool Condition);
    void LDASetStatus();
    void LDXSetStatus();
    void LDYSetStatus();
    void AndSetStatus();
    void ASL(Bus& bus, Byte& Value);
    void LSR(Bus& bus, Byte& Value);
    void ROR(Bus& bus, Byte& Value);
    void ROL(Bus& bus, Byte& Value);
    Word indirectAddrModeX(u32& Cycles, Bus& bus);
    Word indirectAddrModeY(u32& Cycles, Bus& bus, bool addPageCrossCycle = true);
    void startProg(Bus& bus, u32 cycles);
    InstructionHandler GetInstructionHandler(Byte opcode);
    void InvokeInstruction(Byte opcode, u32& Cycles, Bus& bus);
    void Execute(u32& Cycles, Bus& bus);
    void IRQ_Handler(u32& Cycles, Bus& bus, bool Interrupt);
    struct CPUTrace {
    uint16_t pc;
    uint8_t opcode;
    uint8_t op1;
    uint8_t op2;
    uint8_t A;
    uint8_t X;
    uint8_t Y;
    uint8_t P;
    uint8_t SP;
    uint64_t cycles;

    };
    CPUTrace CaptureTrace(Bus& bus) const {
    CPUTrace t;
    t.pc = PC;
    t.opcode = bus.read(PC);
    t.op1 = bus.read(PC + 1);
    t.op2 = bus.read(PC + 2);
    t.A = A;
    t.X = X;
    t.Y = Y;
    t.P = P;
    t.SP = SP;
    return t;
}
};