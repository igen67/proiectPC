#include "cpu.h"
#include "functionHandlers.h"
#include "instructions.h"



void InitializeInstructionTable() {
    // Load/Store Instructions
    CPU::instructionTable[0xA9] = &InstructionHandlers::LDA_IM_Handler;   // LDA Immediate /
    CPU::instructionTable[0xA5] = &InstructionHandlers::LDA_ZP_Handler;   // LDA Zero Page /
    CPU::instructionTable[0xB5] = &InstructionHandlers::LDA_ZPX_Handler;  // LDA Zero Page, X /
    CPU::instructionTable[0xAD] = &InstructionHandlers::LDA_ABS_Handler;  // LDA Absolute /
    CPU::instructionTable[0xBD] = &InstructionHandlers::LDA_ABSX_Handler; // LDA Absolute, X /
    CPU::instructionTable[0xB9] = &InstructionHandlers::LDA_ABSY_Handler; // LDA Absolute, Y / 
    CPU::instructionTable[0xA1] = &InstructionHandlers::LDA_INDX_Handler; // LDA Indirect, X /
    CPU::instructionTable[0xB1] = &InstructionHandlers::LDA_INDY_Handler; // LDA Indirect, Y /

    CPU::instructionTable[0xA2] = &InstructionHandlers::LDX_IM_Handler;   // LDX Immediate
    CPU::instructionTable[0xA6] = &InstructionHandlers::LDX_ZP_Handler;   // LDX Zero Page
    CPU::instructionTable[0xB6] = &InstructionHandlers::LDX_ZPY_Handler;  // LDX Zero Page, Y
    CPU::instructionTable[0xAE] = &InstructionHandlers::LDX_ABS_Handler;  // LDX Absolute
    CPU::instructionTable[0xBE] = &InstructionHandlers::LDX_ABSY_Handler; // LDX Absolute, Y

    CPU::instructionTable[0xA0] = &InstructionHandlers::LDY_IM_Handler;   // LDY Immediate
    CPU::instructionTable[0xA4] = &InstructionHandlers::LDY_ZP_Handler;   // LDY Zero Page
    CPU::instructionTable[0xB4] = &InstructionHandlers::LDY_ZPX_Handler;  // LDY Zero Page, X
    CPU::instructionTable[0xAC] = &InstructionHandlers::LDY_ABS_Handler;  // LDY Absolute
    CPU::instructionTable[0xBC] = &InstructionHandlers::LDY_ABSX_Handler; // LDY Absolute, X

    CPU::instructionTable[0x85] = &InstructionHandlers::STA_ZP_Handler;   // STA Zero Page
    CPU::instructionTable[0x95] = &InstructionHandlers::STA_ZPX_Handler;  // STA Zero Page, X
    CPU::instructionTable[0x8D] = &InstructionHandlers::STA_ABS_Handler;  // STA Absolute
    CPU::instructionTable[0x9D] = &InstructionHandlers::STA_ABSX_Handler; // STA Absolute, X
    CPU::instructionTable[0x99] = &InstructionHandlers::STA_ABSY_Handler; // STA Absolute, Y
    CPU::instructionTable[0x81] = &InstructionHandlers::STA_INDX_Handler; // STA Indirect, X
    CPU::instructionTable[0x91] = &InstructionHandlers::STA_INDY_Handler; // STA Indirect, Y

    CPU::instructionTable[0x86] = &InstructionHandlers::STX_ZP_Handler;   // STX Zero Page
    CPU::instructionTable[0x96] = &InstructionHandlers::STX_ZPY_Handler;  // STX Zero Page, Y
    CPU::instructionTable[0x8E] = &InstructionHandlers::STX_ABS_Handler;  // STX Absolute

    CPU::instructionTable[0x84] = &InstructionHandlers::STY_ZP_Handler;   // STY Zero Page
    CPU::instructionTable[0x94] = &InstructionHandlers::STY_ZPX_Handler;  // STY Zero Page, X
    CPU::instructionTable[0x8C] = &InstructionHandlers::STY_ABS_Handler;  // STY Absolute

    // Register Transfer Instructions
    CPU::instructionTable[0xAA] = &InstructionHandlers::TAX_Handler;      // TAX
    CPU::instructionTable[0x8A] = &InstructionHandlers::TXA_Handler;      // TXA
    CPU::instructionTable[0xA8] = &InstructionHandlers::TAY_Handler;      // TAY
    CPU::instructionTable[0x98] = &InstructionHandlers::TYA_Handler;      // TYA
    CPU::instructionTable[0x9A] = &InstructionHandlers::TXS_Handler;
    CPU::instructionTable[0xBA] = &InstructionHandlers::TSX_Handler;


    // Stack Instructions
    CPU::instructionTable[0x48] = &InstructionHandlers::PHA_Handler;      // PHA
    CPU::instructionTable[0x68] = &InstructionHandlers::PLA_Handler;      // PLA
    CPU::instructionTable[0x08] = &InstructionHandlers::PHP_Handler;      // PHP
    CPU::instructionTable[0x28] = &InstructionHandlers::PLP_Handler;      // PLP

    // Logical and Arithmetic Instructions
    CPU::instructionTable[0x69] = &InstructionHandlers::ADC_IM_Handler;   // ADC Immediate
    CPU::instructionTable[0x65] = &InstructionHandlers::ADC_ZP_Handler;   // ADC Zero Page
    CPU::instructionTable[0x75] = &InstructionHandlers::ADC_ZPX_Handler;  // ADC Zero Page, X
    CPU::instructionTable[0x6D] = &InstructionHandlers::ADC_ABS_Handler;  // ADC Absolute
    CPU::instructionTable[0x7D] = &InstructionHandlers::ADC_ABSX_Handler; // ADC Absolute, X
    CPU::instructionTable[0x79] = &InstructionHandlers::ADC_ABSY_Handler; // ADC Absolute, Y
    CPU::instructionTable[0x61] = &InstructionHandlers::ADC_INDX_Handler; // ADC Indirect, X
    CPU::instructionTable[0x71] = &InstructionHandlers::ADC_INDY_Handler; // ADC Indirect, Y

    CPU::instructionTable[0xE9] = &InstructionHandlers::SBC_IM_Handler;   // SBC Immediate
    CPU::instructionTable[0xE5] = &InstructionHandlers::SBC_ZP_Handler;   // SBC Zero Page
    CPU::instructionTable[0xF5] = &InstructionHandlers::SBC_ZPX_Handler;  // SBC Zero Page, X
    CPU::instructionTable[0xED] = &InstructionHandlers::SBC_ABS_Handler;  // SBC Absolute
    CPU::instructionTable[0xFD] = &InstructionHandlers::SBC_ABSX_Handler; // SBC Absolute, X
    CPU::instructionTable[0xF9] = &InstructionHandlers::SBC_ABSY_Handler; // SBC Absolute, Y
    CPU::instructionTable[0xE1] = &InstructionHandlers::SBC_INDX_Handler; // SBC Indirect, X
    CPU::instructionTable[0xF1] = &InstructionHandlers::SBC_INDY_Handler; // SBC Indirect, Y

    CPU::instructionTable[0x29] = &InstructionHandlers::AND_IM_Handler;   // AND Immediate
    CPU::instructionTable[0x25] = &InstructionHandlers::AND_ZP_Handler;   // AND Zero Page
    CPU::instructionTable[0x35] = &InstructionHandlers::AND_ZPX_Handler;  // AND Zero Page, X
    CPU::instructionTable[0x2D] = &InstructionHandlers::AND_ABS_Handler;  // AND Absolute
    CPU::instructionTable[0x3D] = &InstructionHandlers::AND_ABSX_Handler; // AND Absolute, X
    CPU::instructionTable[0x39] = &InstructionHandlers::AND_ABSY_Handler; // AND Absolute, Y
    CPU::instructionTable[0x21] = &InstructionHandlers::AND_INDX_Handler; // AND Indirect, X
    CPU::instructionTable[0x31] = &InstructionHandlers::AND_INDY_Handler; // AND Indirect, Y

    CPU::instructionTable[0x09] = &InstructionHandlers::ORA_IM_Handler;   // ORA Immediate
    CPU::instructionTable[0x05] = &InstructionHandlers::ORA_ZP_Handler;   // ORA Zero Page
    CPU::instructionTable[0x15] = &InstructionHandlers::ORA_ZPX_Handler;  // ORA Zero Page, X
    CPU::instructionTable[0x0D] = &InstructionHandlers::ORA_ABS_Handler;  // ORA Absolute
    CPU::instructionTable[0x1D] = &InstructionHandlers::ORA_ABSX_Handler; // ORA Absolute, X
    CPU::instructionTable[0x19] = &InstructionHandlers::ORA_ABSY_Handler; // ORA Absolute, Y
    CPU::instructionTable[0x01] = &InstructionHandlers::ORA_INDX_Handler; // ORA Indirect, X

    CPU::instructionTable[0xC9] = &InstructionHandlers::CMP_IM_Handler;
    CPU::instructionTable[0xC5] = &InstructionHandlers::CMP_ZP_Handler;
    CPU::instructionTable[0xD5] = &InstructionHandlers::CMP_ZPX_Handler;
    CPU::instructionTable[0xCD] = &InstructionHandlers::CMP_ABS_Handler;
    CPU::instructionTable[0xDD] = &InstructionHandlers::CMP_ABSX_Handler;
    CPU::instructionTable[0xD9] = &InstructionHandlers::CMP_ABSY_Handler;
    CPU::instructionTable[0xC1] = &InstructionHandlers::CMP_INDX_Handler;
    CPU::instructionTable[0xD1] = &InstructionHandlers::CMP_INDY_Handler;

    CPU::instructionTable[0xE0] = &InstructionHandlers::CPX_IM_Handler; // CPX Immediate
    CPU::instructionTable[0xE4] = &InstructionHandlers::CPX_ZP_Handler; // CPX Zero Page
    CPU::instructionTable[0xEC] = &InstructionHandlers::CPX_ABS_Handler; // CPX Absolute
    CPU::instructionTable[0xC0] = &InstructionHandlers::CPY_IM_Handler; // CPY Immediate
    CPU::instructionTable[0xC4] = &InstructionHandlers::CPY_ZP_Handler; // CPY Zero Page
    CPU::instructionTable[0xCC] = &InstructionHandlers::CPY_ABS_Handler; // CPY Absolute

    CPU::instructionTable[0x49] = &InstructionHandlers::EOR_IM_Handler; 
    CPU::instructionTable[0x45] = &InstructionHandlers::EOR_ZP_Handler;
    CPU::instructionTable[0x55] = &InstructionHandlers::EOR_ZPX_Handler;
    CPU::instructionTable[0x4D] = &InstructionHandlers::EOR_ABS_Handler;
    CPU::instructionTable[0x5D] = &InstructionHandlers::EOR_ABSX_Handler;
    CPU::instructionTable[0x59] = &InstructionHandlers::EOR_ABSY_Handler;
    CPU::instructionTable[0x41] = &InstructionHandlers::EOR_INDX_Handler;
    CPU::instructionTable[0x51] = &InstructionHandlers::EOR_INDY_Handler;



    // Increment and Decrement Instructions
    CPU::instructionTable[0xE6] = &InstructionHandlers::INC_ZP_Handler;   // INC Zero Page
    CPU::instructionTable[0xF6] = &InstructionHandlers::INC_ZPX_Handler;  // INC Zero Page, X
    CPU::instructionTable[0xEE] = &InstructionHandlers::INC_ABS_Handler;  // INC Absolute
    CPU::instructionTable[0xFE] = &InstructionHandlers::INC_ABSX_Handler; // INC Absolute, X

    CPU::instructionTable[0xC6] = &InstructionHandlers::DEC_ZP_Handler;   // DEC Zero Page
    CPU::instructionTable[0xD6] = &InstructionHandlers::DEC_ZPX_Handler;  // DEC Zero Page, X
    CPU::instructionTable[0xCE] = &InstructionHandlers::DEC_ABS_Handler;  // DEC Absolute
    CPU::instructionTable[0xDE] = &InstructionHandlers::DEC_ABSX_Handler; // DEC Absolute, X

    CPU::instructionTable[0xCA] = &InstructionHandlers::DEX_Handler;      // DEX
    CPU::instructionTable[0x88] = &InstructionHandlers::DEY_Handler;      // DEY

    CPU::instructionTable[0xE8] = &InstructionHandlers::INX_Handler;      // INX
    CPU::instructionTable[0xC8] = &InstructionHandlers::INY_Handler;      // INY



    // Shift and Rotate Instructions
    CPU::instructionTable[0x0A] = &InstructionHandlers::ASL_A_Handler;    // ASL Accumulator
    CPU::instructionTable[0x06] = &InstructionHandlers::ASL_ZP_Handler;   // ASL Zero Page
    CPU::instructionTable[0x16] = &InstructionHandlers::ASL_ZPX_Handler;  // ASL Zero Page, X
    CPU::instructionTable[0x0E] = &InstructionHandlers::ASL_ABS_Handler;  // ASL Absolute
    CPU::instructionTable[0x1E] = &InstructionHandlers::ASL_ABSX_Handler; // ASL Absolute, X

    CPU::instructionTable[0x4A] = &InstructionHandlers::LSR_A_Handler;    // LSR Accumulator
    CPU::instructionTable[0x46] = &InstructionHandlers::LSR_ZP_Handler;   // LSR Zero Page
    CPU::instructionTable[0x56] = &InstructionHandlers::LSR_ZPX_Handler;  // LSR Zero Page, X
    CPU::instructionTable[0x4E] = &InstructionHandlers::LSR_ABS_Handler;  // LSR Absolute
    CPU::instructionTable[0x5E] = &InstructionHandlers::LSR_ABSX_Handler; // LSR Absolute, X

    CPU::instructionTable[0x2A] = &InstructionHandlers::ROL_A_Handler;    // ROL Accumulator
    CPU::instructionTable[0x26] = &InstructionHandlers::ROL_ZP_Handler;   // ROL Zero Page
    CPU::instructionTable[0x36] = &InstructionHandlers::ROL_ZPX_Handler;  // ROL Zero Page, X
    CPU::instructionTable[0x2E] = &InstructionHandlers::ROL_ABS_Handler;  // ROL Absolute
    CPU::instructionTable[0x3E] = &InstructionHandlers::ROL_ABSX_Handler; // ROL Absolute, X

    CPU::instructionTable[0x6A] = &InstructionHandlers::ROR_A_Handler;    // ROR Accumulator
    CPU::instructionTable[0x66] = &InstructionHandlers::ROR_ZP_Handler;   // ROR Zero Page
    CPU::instructionTable[0x76] = &InstructionHandlers::ROR_ZPX_Handler;  // ROR Zero Page, X
    CPU::instructionTable[0x6E] = &InstructionHandlers::ROR_ABS_Handler;  // ROR Absolute
    CPU::instructionTable[0x7E] = &InstructionHandlers::ROR_ABSX_Handler; // ROR Absolute, X

    //Branches 
    CPU::instructionTable[0x90] = &InstructionHandlers::BCC_Handler;   // BCC - Branch if Carry Clear
    CPU::instructionTable[0xB0] = &InstructionHandlers::BCS_Handler;   // BCS - Branch if Carry Set
    CPU::instructionTable[0xF0] = &InstructionHandlers::BEQ_Handler;   // BEQ - Branch if Equal (Zero Flag Set)
    CPU::instructionTable[0x30] = &InstructionHandlers::BMI_Handler;   // BMI - Branch if Minus (Negative Flag Set)
    CPU::instructionTable[0xD0] = &InstructionHandlers::BNE_Handler;   // BNE - Branch if Not Equal (Zero Flag Clear)
    CPU::instructionTable[0x10] = &InstructionHandlers::BPL_Handler;   // BPL - Branch if Positive (Negative Flag Clear)
    CPU::instructionTable[0x50] = &InstructionHandlers::BVC_Handler;   // BVC - Branch if Overflow Clear
    CPU::instructionTable[0x70] = &InstructionHandlers::BVS_Handler;   // BVS - Branch if Overflow Set
    

    // Bitwise Test Instructions
    CPU::instructionTable[0x24] = &InstructionHandlers::BIT_ZP_Handler;   // BIT Zero Page
    CPU::instructionTable[0x2C] = &InstructionHandlers::BIT_ABS_Handler;  // BIT Absolute

    // Status Flag Manipulation Instructions 
    CPU::instructionTable[0x18] = &InstructionHandlers::CLC_Handler;      // CLC
    CPU::instructionTable[0xD8] = &InstructionHandlers::CLD_Handler;      // CLD
    CPU::instructionTable[0x58] = &InstructionHandlers::CLI_Handler;      // CLI
    CPU::instructionTable[0xB8] = &InstructionHandlers::CLV_Handler;      // CLV
    CPU::instructionTable[0x38] = &InstructionHandlers::SEC_Handler;      // SEC
    CPU::instructionTable[0xF8] = &InstructionHandlers::SED_Handler;      // SED
    CPU::instructionTable[0x78] = &InstructionHandlers::SEI_Handler;


    //Jump instructions 
    CPU::instructionTable[0x60] = &InstructionHandlers::RTS_Handler;      // RTS
    CPU::instructionTable[0x4C] = &InstructionHandlers::JMP_ABS_Handler;
    CPU::instructionTable[0x6C] = &InstructionHandlers::JMP_IND_Handler;
    CPU::instructionTable[0x20] = &InstructionHandlers::JSR_Handler;

    

    // Other Instructions (NOP, BRK, RTI, RTS)
    CPU::instructionTable[0xEA] = &InstructionHandlers::NOP_Handler;      // NOP
    // unofficial single-byte NOPs sometimes used by assemblers/optimizers
    CPU::instructionTable[0x1A] = &InstructionHandlers::NOP_Handler;      // NOP (0x1A)
    CPU::instructionTable[0x19] = &InstructionHandlers::NOP_Handler;      // NOP (0x19)
    CPU::instructionTable[0x1C] = &InstructionHandlers::NOP_Handler;      // NOP (0x1C)
    CPU::instructionTable[0x1D] = &InstructionHandlers::NOP_Handler;      // NOP (0x1D)
    CPU::instructionTable[0x1E] = &InstructionHandlers::NOP_Handler;      // NOP (0x1E)
    CPU::instructionTable[0x1F] = &InstructionHandlers::NOP_Handler;      // NOP (0x1F)
    CPU::instructionTable[0x00] = &InstructionHandlers::BRK_Handler;      // BRK
    CPU::instructionTable[0x40] = &InstructionHandlers::RTI_Handler;      // RTI
            // Patch: Treat all unknown opcodes as NOP to prevent crashes
    for (int i = 0; i < 256; ++i) {
        if (!CPU::instructionTable[i]) {
            CPU::instructionTable[i] = &InstructionHandlers::NOP_Handler;
        }
    }


}  











