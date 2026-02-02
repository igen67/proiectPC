#pragma once



#include "instructions.h"
#include "bus.h"

using namespace Instructions;

// Global verbose flag to enable CPU trace prints (default off)
extern bool g_verboseCpu;

struct CPU
{
    Word PC; // program counter
    Word SP; // stack pointer

    Byte A, X, Y; // regs

    Byte C : 1; // carry
    Byte Z : 1; // zero
    Byte I : 1; // IR
    Byte D : 1; // decimal
    Byte B : 1; // break
    Byte V : 1; // overflow
    Byte N : 1; // negative

    typedef void (*InstructionHandler)(CPU& cpu, u32& Cycles, Bus& bus);
    static InstructionHandler instructionTable[256];

    bool Interrupt = false;
    // NMI request (set by PPU on VBlank)
    bool NMIRequested = false;
    // Tracing: when >0, CPU will print executed instructions and bus interactions for debugging
    int traceInstructionsRemaining = 0;
    void HandleNMI(u32& Cycles, Bus& bus);

    void printReg(char reg);
    void modifySP();
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
    Word indirectAddrModeY(u32& Cycles, Bus& bus);
    void startProg(Bus& bus, u32 cycles);
    InstructionHandler GetInstructionHandler(Byte opcode);
    void InvokeInstruction(Byte opcode, u32& Cycles, Bus& bus);
    void Step(u32& Cycles, Bus& bus); // Execute a single instruction
    void Execute(u32& Cycles, Bus& bus);
    void IRQ_Handler(u32& Cycles, Bus& bus, bool Interrupt);
};