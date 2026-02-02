#pragma once

#include "cpu.h"
#include "bus.h"
#include "types.h"



struct InstructionHandlers
{
    //LDA INSTRUCTIONS
    // Immediate: 2 cycles
    static void LDA_IM_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.A = cpu.FetchByte(Cycles, bus);
        cpu.LDASetStatus();
        Cycles += 2;
    }
    // Zero Page: 3 cycles
    static void LDA_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.A = bus.read(cpu.FetchByte(Cycles, bus));
        cpu.LDASetStatus();
        Cycles += 3;
    }
    // Zero Page,X: 4 cycles
    static void LDA_ZPX_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.A = bus.read(cpu.FetchByte(Cycles, bus) + cpu.X);
        cpu.LDASetStatus();
        Cycles += 4;
    }
    // Absolute: 4 cycles
    static void LDA_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Word Address = cpu.FetchWord(Cycles, bus);
        cpu.A = bus.read(Address);
        cpu.LDASetStatus();
        Cycles += 4;
    }
    // Absolute,X: 4 cycles (+1 if page crossed)
    static void LDA_ABSX_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Word Address = cpu.FetchWord(Cycles, bus);
        bool pageCrossed = ((Address & 0xFF00) != ((Address + cpu.X) & 0xFF00));
        cpu.A = bus.read(Address + cpu.X);
        cpu.LDASetStatus();
        Cycles += 4 + (pageCrossed ? 1 : 0);
    }
    // Absolute,Y: 4 cycles (+1 if page crossed)
    static void LDA_ABSY_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Word Address = cpu.FetchWord(Cycles, bus);
        bool pageCrossed = ((Address & 0xFF00) != ((Address + cpu.Y) & 0xFF00));
        cpu.A = bus.read(Address + cpu.Y);
        cpu.LDASetStatus();
        Cycles += 4 + (pageCrossed ? 1 : 0);
    }
    // Indirect,X: 6 cycles
    static void LDA_INDX_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.A = bus.read(cpu.indirectAddrModeX(Cycles, bus));
        cpu.LDASetStatus();
        Cycles += 6;
    }
    // Indirect,Y: 5 cycles (+1 if page crossed)
    static void LDA_INDY_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Word addr = cpu.indirectAddrModeY(Cycles, bus);
        bool pageCrossed = ((addr & 0xFF00) != ((addr) & 0xFF00)); // You may want to check page crossing with Y
        cpu.A = bus.read(addr);
        cpu.LDASetStatus();
        Cycles += 5 + (pageCrossed ? 1 : 0);
    }




    //STA INSTRUCTIONS
    // Zero Page: 3 cycles
    static void STA_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Byte Address = cpu.FetchByte(Cycles, bus);
        bus.write(Address, cpu.A);
        Cycles += 3;
    }
    // Zero Page,X: 4 cycles
    static void STA_ZPX_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Byte Address = cpu.FetchByte(Cycles, bus) + cpu.X;
        bus.write(Address, cpu.A);
        Cycles += 4;
    }
    // Absolute: 4 cycles
    static void STA_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Word Address = cpu.FetchWord(Cycles, bus);
        bus.write(Address, cpu.A);
        Cycles += 4;
    }
    // Absolute,X: 5 cycles
    static void STA_ABSX_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Word Address = cpu.FetchWord(Cycles, bus) + cpu.X;
        bus.write(Address, cpu.A);
        Cycles += 5;
    }
    // Absolute,Y: 5 cycles
    static void STA_ABSY_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Word Address = cpu.FetchWord(Cycles, bus) + cpu.Y;
        bus.write(Address, cpu.A);
        Cycles += 5;
    }
    // Indirect,X: 6 cycles
    static void STA_INDX_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        bus.write(cpu.indirectAddrModeX(Cycles, bus), cpu.A);
        Cycles += 6;
    }
    // Indirect,Y: 6 cycles
    static void STA_INDY_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        bus.write(cpu.indirectAddrModeY(Cycles, bus), cpu.A);
        Cycles += 6;
    }
    //STX INSTRUCTIONS
    // Zero Page: 3 cycles
    static void STX_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Byte Address = cpu.FetchByte(Cycles, bus);
        bus.write(Address, cpu.X);
        Cycles += 3;
    }
    // Zero Page,Y: 4 cycles
    static void STX_ZPY_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Byte Address = cpu.FetchByte(Cycles, bus) + cpu.Y;
        bus.write(Address, cpu.X);
        Cycles += 4;
    }
    // Absolute: 4 cycles
    static void STX_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Word Address = cpu.FetchWord(Cycles, bus);
        bus.write(Address, cpu.X);
        Cycles += 4;
    }
    //STY INSTRUCTIONS
    // Zero Page: 3 cycles
    static void STY_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Byte Address = cpu.FetchByte(Cycles, bus);
        bus.write(Address, cpu.Y);
        Cycles += 3;
    }
    // Zero Page,X: 4 cycles
    static void STY_ZPX_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Byte Address = cpu.FetchByte(Cycles, bus) + cpu.X;
        bus.write(Address, cpu.Y);
        Cycles += 4;
    }
    // Absolute: 4 cycles
    static void STY_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Word Address = cpu.FetchWord(Cycles, bus);
        bus.write(Address, cpu.Y);
        Cycles += 4;
    }
    //LDX INSTRUCTIONS
    // Immediate: 2 cycles
    static void LDX_IM_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.X = cpu.FetchByte(Cycles, bus);
        cpu.LDXSetStatus();
        Cycles += 2;
    }
    // Zero Page: 3 cycles
    static void LDX_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.X = bus.read(cpu.FetchByte(Cycles, bus));
        cpu.LDXSetStatus();
        Cycles += 3;
    }
    // Zero Page,Y: 4 cycles
    static void LDX_ZPY_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.X = bus.read(cpu.FetchByte(Cycles, bus) + cpu.Y);
        cpu.LDXSetStatus();
        Cycles += 4;
    }
    // Absolute: 4 cycles
    static void LDX_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.X = bus.read(cpu.FetchWord(Cycles, bus));
        cpu.LDXSetStatus();
        Cycles += 4;
    }
    // Absolute,Y: 4 cycles (+1 if page crossed)
    static void LDX_ABSY_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Word Address = cpu.FetchWord(Cycles, bus);
        bool pageCrossed = ((Address & 0xFF00) != ((Address + cpu.Y) & 0xFF00));
        cpu.X = bus.read(Address + cpu.Y);
        cpu.LDXSetStatus();
        Cycles += 4 + (pageCrossed ? 1 : 0);
    }
    //LDY INSTRUCTIONS
    // Immediate: 2 cycles
    static void LDY_IM_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.Y = cpu.FetchByte(Cycles, bus);
        cpu.LDYSetStatus();
        Cycles += 2;
    }
    // Zero Page: 3 cycles
    static void LDY_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.Y = bus.read(cpu.FetchByte(Cycles, bus));
        cpu.LDYSetStatus();
        Cycles += 3;
    }
    // Zero Page,X: 4 cycles
    static void LDY_ZPX_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.Y = bus.read(cpu.FetchByte(Cycles, bus) + cpu.X);
        cpu.LDYSetStatus();
        Cycles += 4;
    }
    // Absolute: 4 cycles
    static void LDY_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.Y = bus.read(cpu.FetchWord(Cycles, bus));
        cpu.LDYSetStatus();
        Cycles += 4;
    }
    // Absolute,X: 4 cycles (+1 if page crossed)
    static void LDY_ABSX_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Word Address = cpu.FetchWord(Cycles, bus);
        bool pageCrossed = ((Address & 0xFF00) != ((Address + cpu.X) & 0xFF00));
        cpu.Y = bus.read(Address + cpu.X);
        cpu.LDYSetStatus();
        Cycles += 4 + (pageCrossed ? 1 : 0);
    }
    //REGISTER INSTRUCTIONS
    // All implied, 2 cycles
    static void TAX_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.X = cpu.A;
        cpu.Z = (cpu.X == 0);
        cpu.N = (cpu.X & 0b10000000) > 0;
        Cycles += 2;
    }
    static void TAY_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.Y = cpu.A;
        cpu.Z = (cpu.Y == 0);
        cpu.N = (cpu.Y & 0b10000000) > 0;
        cpu.printReg(cpu.Y);
        Cycles += 2;
    }
    static void TYA_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.A = cpu.Y;
        cpu.Z = (cpu.A == 0);
        cpu.N = (cpu.A & 0b10000000) > 0;
        Cycles += 2;
    }
    static void TXA_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.A = cpu.X;
        cpu.Z = (cpu.A == 0);
        cpu.N = (cpu.A & 0b10000000) > 0;
        Cycles += 2;
    }
    static void TXS_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        // TXS: 2 cycles
        if (cpu.X & (1 << 0)) cpu.C = 1;
        if (cpu.X & (1 << 1)) cpu.Z = 1;
        if (cpu.X & (1 << 2)) cpu.I = 1;
        if (cpu.X & (1 << 3)) cpu.D = 1;
        if (cpu.X & (1 << 4)) cpu.B = 1;
        if (cpu.X & (1 << 6)) cpu.V = 1;
        if (cpu.X & (1 << 7)) cpu.N = 1;
        Cycles += 2;
    }
    static void TSX_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        // TSX: 2 cycles
        if (cpu.C == 1) cpu.X = cpu.X | 0b00000001;
        if (cpu.Z == 1) cpu.X = cpu.X | 0b00000010;
        if (cpu.I == 1) cpu.X = cpu.X | 0b00000100;
        if (cpu.D == 1) cpu.X = cpu.X | 0b00001000;
        if (cpu.B == 1) cpu.X = cpu.X | 0b00010000;
        if (cpu.V == 1) cpu.X = cpu.X | 0b01000000;
        if (cpu.N == 1) cpu.X = cpu.X | 0b10000000;
        Cycles += 2;
    }


    //MEMORY MANIP INSTRUCTIONS
    static void DEC_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte Address = cpu.FetchByte(Cycles, bus);
        Byte val = bus.read(Address);
        val -= 1;
        bus.write(Address, val);
        cpu.Z = (val == 0);
        cpu.N = (val & 0b10000000) > 0;
        Cycles += 5;
    }
    static void DEC_ZPX_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte Address = cpu.FetchByte(Cycles, bus)+cpu.X;
        Byte val = bus.read(Address);
        val -= 1;
        bus.write(Address, val);
        cpu.Z = (val == 0);
        cpu.N = (val & 0b10000000) > 0;
        Cycles += 6;
    }

    static void DEC_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Word Address = cpu.FetchWord(Cycles, bus);
        Byte val = bus.read(Address);
        val -= 1;
        bus.write(Address, val);
        cpu.Z = (val == 0);
        cpu.N = (val & 0b10000000) > 0;
        Cycles += 6;
    }

    static void DEC_ABSX_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Word Address = cpu.FetchWord(Cycles, bus)+cpu.X;
        Byte val = bus.read(Address);
        val -= 1;
        bus.write(Address, val);
        cpu.Z = (val == 0);
        cpu.N = (val & 0b10000000) > 0;
        Cycles += 7;
    }


    static void INC_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Byte Address = cpu.FetchByte(Cycles, bus);
        Byte val = bus.read(Address);
        val += 1;
        bus.write(Address, val);
        cpu.Z = (val == 0);
        cpu.N = (val & 0b10000000) > 0;
        Cycles += 5;
    }
    static void INC_ZPX_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte Address = cpu.FetchByte(Cycles, bus)+cpu.X;
        Byte val = bus.read(Address);
        val += 1;
        bus.write(Address, val);
        cpu.Z = (val == 0);
        cpu.N = (val & 0b10000000) > 0;
        Cycles += 6;
    }

    static void INC_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Word Address = cpu.FetchWord(Cycles, bus);
        Byte val = bus.read(Address);
        val += 1;
        bus.write(Address, val);
        cpu.Z = (val == 0);
        cpu.N = (val & 0b10000000) > 0;
        Cycles += 6;
    }

    static void INC_ABSX_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Word Address = cpu.FetchWord(Cycles, bus)+cpu.X;
        Byte val = bus.read(Address);
        val += 1;
        bus.write(Address, val);
        cpu.Z = (val == 0);
        cpu.N = (val & 0b10000000) > 0;
        Cycles += 7;
    }














    //ALGEBRA AND LOGIC INSTRUCIONS


    static void SBC_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        cpu.SBCSetStatus(bus.read(cpu.FetchByte(Cycles, bus)));
    }
    static void SBC_IM_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.SBCSetStatus(cpu.FetchByte(Cycles, bus));
    }
    static void SBC_ZPX_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.SBCSetStatus(bus.read(cpu.FetchByte(Cycles, bus)+cpu.X));
    }
    static void SBC_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.SBCSetStatus(bus.read(cpu.FetchWord(Cycles, bus)));
    }
    static void SBC_ABSX_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.SBCSetStatus(bus.read(cpu.FetchWord(Cycles, bus)+cpu.X));
    }
    static void SBC_ABSY_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.SBCSetStatus(bus.read(cpu.FetchWord(Cycles, bus)+cpu.Y));
    }
    static void SBC_INDX_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.SBCSetStatus(bus.read(cpu.indirectAddrModeX(Cycles, bus)));
    }
    static void SBC_INDY_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.SBCSetStatus(bus.read(cpu.indirectAddrModeY(Cycles, bus)));
    }


    static void AND_IM_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.A = cpu.A & cpu.FetchByte(Cycles, bus);
        cpu.AndSetStatus();
        Cycles += 2;
    }
    static void AND_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A & bus.read(cpu.FetchByte(Cycles, bus));
        cpu.AndSetStatus();
        Cycles += 3;
    }
    static void AND_ZPX_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A & bus.read(cpu.FetchByte(Cycles, bus)+cpu.X);
        cpu.AndSetStatus();
        Cycles += 4;
    }
    static void AND_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A & bus.read(cpu.FetchWord(Cycles, bus));
        cpu.AndSetStatus();
        Cycles += 4;
    }
    static void AND_ABSX_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A & bus.read(cpu.FetchWord(Cycles, bus)+cpu.X);
        cpu.AndSetStatus();
        Cycles += 4;
    }
    static void AND_ABSY_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A & bus.read(cpu.FetchWord(Cycles, bus)+cpu.Y);
        cpu.AndSetStatus();
        Cycles += 4;
    }
    static void AND_INDX_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A & bus.read(cpu.indirectAddrModeX(Cycles, bus));
        cpu.AndSetStatus();
        Cycles += 6;
    }
    static void AND_INDY_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A & bus.read(cpu.indirectAddrModeY(Cycles, bus));
        cpu.AndSetStatus();
        Cycles += 5;
    }

    static void ORA_IM_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A | cpu.FetchByte(Cycles, bus);
        cpu.AndSetStatus();
        Cycles += 2;
    }
    static void ORA_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A | bus.read(cpu.FetchByte(Cycles, bus));
        cpu.AndSetStatus();
        Cycles += 3;
    }
    static void ORA_ZPX_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A | bus.read(cpu.FetchByte(Cycles, bus)+cpu.X);
        cpu.AndSetStatus();
        Cycles += 4;
    }
    static void ORA_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A | bus.read(cpu.FetchWord(Cycles, bus));
        cpu.AndSetStatus();
        Cycles += 4;
    }
    static void ORA_ABSX_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A | bus.read(cpu.FetchWord(Cycles, bus)+cpu.X);
        cpu.AndSetStatus();
        Cycles += 4;
    }
    static void ORA_ABSY_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A | bus.read(cpu.FetchWord(Cycles, bus)+cpu.Y);
        cpu.AndSetStatus();
        Cycles += 4;
    }
    static void ORA_INDX_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A | bus.read(cpu.indirectAddrModeX(Cycles, bus));
        cpu.AndSetStatus();
        Cycles += 6;
    }
    static void ORA_INDY_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A | bus.read(cpu.indirectAddrModeY(Cycles, bus));
        cpu.AndSetStatus();
        Cycles += 5;
    }

    static void EOR_IM_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A ^ cpu.FetchByte(Cycles, bus);
        cpu.AndSetStatus();
        Cycles += 2;
    }
    static void EOR_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A ^ bus.read(cpu.FetchByte(Cycles, bus));
        cpu.AndSetStatus();
        Cycles += 3;
    }
    static void EOR_ZPX_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A ^ bus.read(cpu.FetchByte(Cycles, bus)+cpu.X);
        cpu.AndSetStatus();
        Cycles += 4;
    }
    static void EOR_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A ^ bus.read(cpu.FetchWord(Cycles, bus));
        cpu.AndSetStatus();
        Cycles += 4;
    }
    static void EOR_ABSX_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A ^ bus.read(cpu.FetchWord(Cycles, bus)+cpu.X);
        cpu.AndSetStatus();
        Cycles += 4;
    }
    static void EOR_ABSY_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A ^ bus.read(cpu.FetchWord(Cycles, bus)+cpu.Y);
        cpu.AndSetStatus();
        Cycles += 4;
    }
    static void EOR_INDX_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A ^ bus.read(cpu.indirectAddrModeX(Cycles, bus));
        cpu.AndSetStatus();
        Cycles += 6;
    }
    static void EOR_INDY_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A ^ bus.read(cpu.indirectAddrModeY(Cycles, bus));
        cpu.AndSetStatus();
        Cycles += 5;
    }


    static void CMP_IM_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte Value = cpu.FetchByte(Cycles, bus);
        Byte Temp = cpu.A - Value;
        cpu.C = (cpu.A >= Value);
        cpu.Z = (Temp == 0x00);
        cpu.N = (Temp & 0b10000000) > 0;
        if (g_verboseCpu) {
            printf("CMP: A = 0x%02X, Value = 0x%02X, Z = %d, C = %d, N = %d\n", cpu.A, Value, cpu.Z, cpu.C, cpu.N);
        }
    }
    static void CMP_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte Value = bus.read(cpu.FetchByte(Cycles, bus));
        Byte Temp = cpu.A - Value;
        cpu.C = (cpu.A >= Value);
        cpu.Z = (Temp == 0x00);
        cpu.N = (Temp & 0b10000000) > 0;
        if (g_verboseCpu) {
            printf("CMP: A = 0x%02X, Value = 0x%02X, Z = %d, C = %d, N = %d\n", cpu.A, Value, cpu.Z, cpu.C, cpu.N);
        }
    }
    static void CMP_ZPX_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte Value = bus.read(cpu.FetchByte(Cycles, bus)+cpu.X);
        Byte Temp = cpu.A - Value;
        cpu.C = (cpu.A >= Value);
        cpu.Z = (Temp == 0x00);
        cpu.N = (Temp & 0b10000000) > 0;
        if (g_verboseCpu) {
            printf("CMP: A = 0x%02X, Value = 0x%02X, Z = %d, C = %d, N = %d\n", cpu.A, Value, cpu.Z, cpu.C, cpu.N);
        }
    }
    static void CMP_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte Value = bus.read(cpu.FetchWord(Cycles, bus));
        Byte Temp = cpu.A - Value;
        cpu.C = (cpu.A >= Value);
        cpu.Z = (Temp == 0x00);
        cpu.N = (Temp & 0b10000000) > 0;
        if (g_verboseCpu) {
            printf("CMP: A = 0x%02X, Value = 0x%02X, Z = %d, C = %d, N = %d\n", cpu.A, Value, cpu.Z, cpu.C, cpu.N);
        }
    }
    static void CMP_ABSX_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte Value = bus.read(cpu.FetchWord(Cycles, bus)+cpu.X);
        Byte Temp = cpu.A - Value;
        cpu.C = (cpu.A >= Value);
        cpu.Z = (Temp == 0x00);
        cpu.N = (Temp & 0b10000000) > 0;
        if (g_verboseCpu) {
            printf("CMP: A = 0x%02X, Value = 0x%02X, Z = %d, C = %d, N = %d\n", cpu.A, Value, cpu.Z, cpu.C, cpu.N);
        }
    }
    static void CMP_ABSY_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte Value = bus.read(cpu.FetchWord(Cycles, bus)+cpu.Y);
        Byte Temp = cpu.A - Value;
        cpu.C = (cpu.A >= Value);
        cpu.Z = (Temp == 0x00);
        cpu.N = (Temp & 0b10000000) > 0;
        if (g_verboseCpu) {
            printf("CMP: A = 0x%02X, Value = 0x%02X, Z = %d, C = %d, N = %d\n", cpu.A, Value, cpu.Z, cpu.C, cpu.N);
        }
    }
    static void CMP_INDX_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte Value = bus.read(cpu.indirectAddrModeX(Cycles, bus));
        Byte Temp = cpu.A - Value;
        cpu.C = (cpu.A >= Value);
        cpu.Z = (Temp == 0x00);
        cpu.N = (Temp & 0b10000000) > 0;
        if (g_verboseCpu) {
            printf("CMP: A = 0x%02X, Value = 0x%02X, Z = %d, C = %d, N = %d\n", cpu.A, Value, cpu.Z, cpu.C, cpu.N);
        }
    }
    static void CMP_INDY_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte Value = bus.read(cpu.indirectAddrModeY(Cycles, bus));
        Byte Temp = cpu.A - Value;
        cpu.C = (cpu.A >= Value);
        cpu.Z = (Temp == 0x00);
        cpu.N = (Temp & 0b10000000) > 0;
        if (g_verboseCpu) {
            printf("CMP: A = 0x%02X, Value = 0x%02X, Z = %d, C = %d, N = %d\n", cpu.A, Value, cpu.Z, cpu.C, cpu.N);
        }
    }

    static void CPX_IM_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        Byte Value = cpu.FetchByte(Cycles, bus);
        Byte Temp = cpu.X - Value;
        cpu.C = (cpu.X >= Value);
        cpu.Z = (Temp == 0x00);
        cpu.N = (Temp & 0b10000000) > 0;
    }

    static void CPX_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        Byte Value = bus.read(cpu.FetchByte(Cycles, bus));
        Byte Temp = cpu.X - Value;
        cpu.C = (cpu.X >= Value);
        cpu.Z = (Temp == 0x00);
        cpu.N = (Temp & 0b10000000) > 0;
    }

    static void CPX_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        Byte Value = bus.read(cpu.FetchWord(Cycles, bus));
        Byte Temp = cpu.X - Value;
        cpu.C = (cpu.X >= Value);
        cpu.Z = (Temp == 0x00);
        cpu.N = (Temp & 0b10000000) > 0;
    }

    static void CPY_IM_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        Byte Value = cpu.FetchByte(Cycles, bus);
        Byte Temp = cpu.Y - Value;
        cpu.C = (cpu.Y >= Value);
        cpu.Z = (Temp == 0x00);
        cpu.N = (Temp & 0b10000000) > 0;
    }

    static void CPY_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        Byte Value = bus.read(cpu.FetchByte(Cycles, bus));
        Byte Temp = cpu.Y - Value;
        cpu.C = (cpu.Y >= Value);
        cpu.Z = (Temp == 0x00);
        cpu.N = (Temp & 0b10000000) > 0;
    }

    static void CPY_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        Byte Value = bus.read(cpu.FetchWord(Cycles, bus));
        Byte Temp = cpu.Y - Value;
        cpu.C = (cpu.Y >= Value);
        cpu.Z = (Temp == 0x00);
        cpu.N = (Temp & 0b10000000) > 0;
    }




    static void ADC_IM_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.ADCSetStatus(cpu.FetchByte(Cycles, bus));
    }

    static void ADC_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.ADCSetStatus(bus.read(cpu.FetchByte(Cycles, bus)));
        Cycles += 3;
    }
    static void ADC_ZPX_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.ADCSetStatus(bus.read(cpu.FetchByte(Cycles, bus)+cpu.X));
        Cycles += 4;
    }
    static void ADC_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.ADCSetStatus(bus.read(cpu.FetchWord(Cycles, bus)));
    }
    static void ADC_ABSX_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.ADCSetStatus(bus.read(cpu.FetchWord(Cycles, bus)+cpu.X));
    }
    static void ADC_ABSY_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.ADCSetStatus(bus.read(cpu.FetchWord(Cycles, bus)+cpu.Y));

    }
    static void ADC_INDX_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.ADCSetStatus(bus.read(cpu.indirectAddrModeX(Cycles, bus)));
        Cycles += 6;
    }
    static void ADC_INDY_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.ADCSetStatus(bus.read(cpu.indirectAddrModeY(Cycles, bus)));
        Cycles += 5; // +1 if page crossed handled in addr mode
    }

    //SHIFT AND ROTATE
    static void ASL_A_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        cpu.ASL(bus, cpu.A);
        Cycles += 2;
    }

    static void ASL_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte addr = cpu.FetchByte(Cycles, bus);
        Byte val = bus.read(addr);
        cpu.ASL(bus, val);
        bus.write(addr, val);
    }

    static void ASL_ZPX_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte addr = cpu.FetchByte(Cycles, bus)+cpu.X;
        Byte val = bus.read(addr);
        cpu.ASL(bus, val);
        bus.write(addr, val);
    }

    static void ASL_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Word addr = cpu.FetchWord(Cycles, bus);
        Byte val = bus.read(addr);
        cpu.ASL(bus, val);
        bus.write(addr, val);
    }

    static void ASL_ABSX_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
            Word addr = cpu.FetchWord(Cycles, bus)+cpu.X;
            Byte val = bus.read(addr);
            cpu.ASL(bus, val);
            bus.write(addr, val);
    }

    static void LSR_A_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        cpu.LSR(bus, cpu.A);
        Cycles += 2;
    }

    static void LSR_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte addr = cpu.FetchByte(Cycles, bus);
        Byte val = bus.read(addr);
        cpu.LSR(bus, val);
        bus.write(addr, val);
        Cycles += 5;
    }

    static void LSR_ZPX_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte addr = cpu.FetchByte(Cycles, bus)+cpu.X;
        Byte val = bus.read(addr);
        cpu.LSR(bus, val);
        bus.write(addr, val);
        Cycles += 6;
    }

    static void LSR_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Word addr = cpu.FetchWord(Cycles, bus);
        Byte val = bus.read(addr);
        cpu.LSR(bus, val);
        bus.write(addr, val);
    }

    static void LSR_ABSX_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Word addr = cpu.FetchWord(Cycles, bus)+cpu.X;
        Byte val = bus.read(addr);
        cpu.LSR(bus, val);
        bus.write(addr, val);
    }

    static void ROL_A_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        cpu.ROL(bus, cpu.A);
        Cycles += 2;
    }

    static void ROL_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte addr = cpu.FetchByte(Cycles, bus);
        Byte val = bus.read(addr);
        cpu.ROL(bus, val);
        bus.write(addr, val);
        Cycles += 5;
    }

    static void ROL_ZPX_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte addr = cpu.FetchByte(Cycles, bus)+cpu.X;
        Byte val = bus.read(addr);
        cpu.ROL(bus, val);
        bus.write(addr, val);
        Cycles += 6;
    }

    static void ROL_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Word addr = cpu.FetchWord(Cycles, bus);
        Byte val = bus.read(addr);
        cpu.ROL(bus, val);
        bus.write(addr, val);
        Cycles += 6;
    }

    static void ROL_ABSX_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Word addr = cpu.FetchWord(Cycles, bus)+cpu.X;
        Byte val = bus.read(addr);
        cpu.ROL(bus, val);
        bus.write(addr, val);
        Cycles += 7;
    }

    static void ROR_A_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        cpu.ROR(bus, cpu.A);
        Cycles += 2;
    }

    static void ROR_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte addr = cpu.FetchByte(Cycles, bus);
        Byte val = bus.read(addr);
        cpu.ROR(bus, val);
        bus.write(addr, val);
        Cycles += 5;
    }

    static void ROR_ZPX_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte addr = cpu.FetchByte(Cycles, bus)+cpu.X;
        Byte val = bus.read(addr);
        cpu.ROR(bus, val);
        bus.write(addr, val);
        Cycles += 6;
    }

    static void ROR_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Word addr = cpu.FetchWord(Cycles, bus);
        Byte val = bus.read(addr);
        cpu.ROR(bus, val);
        bus.write(addr, val);
        Cycles += 6;
    }

    static void ROR_ABSX_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Word addr = cpu.FetchWord(Cycles, bus)+cpu.X;
        Byte val = bus.read(addr);
        cpu.ROR(bus, val);
        bus.write(addr, val);
        Cycles += 7;
    }





    //REGISTER MANIP
    static void DEX_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        cpu.X -= 0x01;
        cpu.Z = (cpu.X == 0);
        cpu.N = (cpu.X & 0b10000000) > 0;
        Cycles += 2;
    }

       static void INX_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.X += 1;
        cpu.C = (cpu.X > 0xFF);
        Cycles += 2;
    }
        static void DEY_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.Y -= 0x01;
        cpu.Z = (cpu.Y == 0);
        cpu.N = (cpu.Y & 0b10000000) > 0;
        Cycles += 2;
    }
    static void INY_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.Y += 1;
        cpu.Z = (cpu.Y == 0);
        cpu.N = (cpu.Y & 0b10000000) > 0;
        cpu.C = (cpu.Y > 0xFF);
        Cycles += 2;
    }





    //BRANCH INSTRUCTIONS
    static void BCC_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.ExecuteBranch(Cycles, bus, !cpu.C); // Handles cycles internally
    }

    static void BCS_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.ExecuteBranch(Cycles, bus, cpu.C);
    }

    static void BEQ_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.ExecuteBranch(Cycles, bus, cpu.Z);
    }

    static void BMI_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.ExecuteBranch(Cycles, bus, cpu.N);
    }

    static void BNE_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.ExecuteBranch(Cycles, bus, !cpu.Z);
    }

    static void BPL_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.ExecuteBranch(Cycles, bus, !cpu.N);
    }

    static void BVC_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.ExecuteBranch(Cycles, bus, !cpu.V);
    }

    static void BVS_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.ExecuteBranch(Cycles, bus, cpu.V);
    }
    //STACK INSTRUCTIONS
    // PHA: 3 cycles
    static void PHA_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        // Push A onto the stack (stack is on page 1, wraps via modifySP)
        cpu.SP--;
        cpu.modifySP();
        bus.write(cpu.SP, cpu.A);
        Cycles += 3;
    }
    // PLA: 4 cycles
    static void PLA_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        if (cpu.SP == 0x01FF) {
            std::cerr << "Stack overflow!" << std::endl;
            exit(1);
        }
        cpu.A = bus.read(cpu.SP);
        cpu.SP++;
        cpu.modifySP();
        Cycles += 4;
    }
    // PHP: 3 cycles
    static void PHP_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.SP--;
        cpu.modifySP();
        Byte status = 0;
        if (cpu.C == 1) status |= 0b00000001;
        if (cpu.Z == 1) status |= 0b00000010;
        if (cpu.I == 1) status |= 0b00000100;
        if (cpu.D == 1) status |= 0b00001000;
        if (cpu.B == 1) status |= 0b00010000;
        if (cpu.V == 1) status |= 0b01000000;
        if (cpu.N == 1) status |= 0b10000000;
        bus.write(cpu.SP, status);
        Cycles += 3;
    }
    // PLP: 4 cycles
    static void PLP_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.SP++;
        cpu.modifySP();
        Byte status = bus.read(cpu.SP);
        cpu.C = (status & (1 << 0)) ? 1 : 0;
        cpu.Z = (status & (1 << 1)) ? 1 : 0;
        cpu.I = (status & (1 << 2)) ? 1 : 0;
        cpu.D = (status & (1 << 3)) ? 1 : 0;
        cpu.B = (status & (1 << 4)) ? 1 : 0;
        cpu.V = (status & (1 << 6)) ? 1 : 0;
        cpu.N = (status & (1 << 7)) ? 1 : 0;
        Cycles += 4;
    }
    //FLAG INSTRUCTIONS
    // All implied, 2 cycles
    static void CLC_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.C = 0;
        Cycles += 2;
    }
    static void CLD_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.D = 0;
        Cycles += 2;
    }
    static void CLI_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.I = 0;
        Cycles += 2;
    }
    static void CLV_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.V = 0;
        Cycles += 2;
    }
    static void SEC_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.C = 1;
        Cycles += 2;
    }
    static void SED_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.D = 1;
        Cycles += 2;
    }
    static void SEI_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.I = 1;
        Cycles += 2;
    }
    //JUMP INSTRUCTIONS
    // JMP Absolute: 3 cycles
    static void JMP_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.PC = cpu.FetchWord(Cycles, bus);
        // debug: printf("JMP to 0x%X\n", cpu.PC);
        Cycles += 3;
    }
    // JMP Indirect: 5 cycles
    static void JMP_IND_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Word ptr = cpu.FetchWord(Cycles, bus);
        Byte low = bus.read(ptr);
        Word highAddr;
        if ((ptr & 0x00FF) == 0x00FF) {
            // 6502 page-wrap bug
            highAddr = ptr & 0xFF00;
        } else {
            highAddr = ptr + 1;
        }
        Byte high = bus.read(highAddr);
        cpu.PC = (high << 8) | low;
        Cycles += 5;
    }
    // JSR: 6 cycles
    static void JSR_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        // On entry, PC points to the low byte of the operand (FetchByte advanced PC)
        // JSR should push (address of return - 1) so that RTS pulling it and adding 1 returns to the byte after JSR.
        Word returnAddress = cpu.PC + 2; // address of the last byte of the JSR instruction
        // push high then low bytes
        bus.write(cpu.SP, (returnAddress >> 8) & 0xFF);
        cpu.SP--;
        cpu.modifySP();
        bus.write(cpu.SP, returnAddress & 0xFF);
        cpu.SP--;
        cpu.modifySP();
        cpu.PC = cpu.FetchWord(Cycles, bus);
        Cycles += 6;
    }
    // RTS: 6 cycles
    static void RTS_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.SP++;
        Byte lo = bus.read(0x0100 | cpu.SP);
        cpu.SP++;
        Byte hi = bus.read(0x0100 | cpu.SP);
        Cycles += 6;
    }
    // RTI: 6 cycles
    static void RTI_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.SP++;
        Byte status = bus.read(0x0100 | cpu.SP);

        cpu.C = status & 0x01;
        cpu.Z = status & 0x02;
        cpu.I = status & 0x04;
        cpu.D = status & 0x08;
        // bit 4 ignored
        // bit 5 ignored
        cpu.V = status & 0x40;
        cpu.N = status & 0x80;

        cpu.SP++;
        Byte lo = bus.read(0x0100 | cpu.SP);

        cpu.SP++;
        Byte hi = bus.read(0x0100 | cpu.SP);

        cpu.PC = (hi << 8) | lo;
        Cycles += 6;
        printf("STATUS=%02X I=%d\n", status, cpu.I);

    }
    // BRK: 7 cycles
    static void BRK_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Word returnAddress = cpu.PC + 2;
        // push high then low
        cpu.SP--;
        cpu.modifySP();
        bus.write(cpu.SP, (returnAddress >> 8) & 0xFF);
        cpu.SP--;
        cpu.modifySP();
        bus.write(cpu.SP, returnAddress & 0xFF);
        // push status with B flag set
        cpu.SP--;
        cpu.modifySP();
        Byte status = 0;
        if (cpu.C == 1) status |= 0b00000001;
        if (cpu.Z == 1) status |= 0b00000010;
        if (cpu.I == 1) status |= 0b00000100;
        if (cpu.D == 1) status |= 0b00001000;
        status |= 0b00010000; // B flag
        status |= 0b00100000;
        if (cpu.V == 1) status |= 0b01000000;
        if (cpu.N == 1) status |= 0b10000000;
        bus.write(cpu.SP, status);
        cpu.I = 0x01;
        cpu.PC = bus.read(0xFFFE) | (bus.read(0xFFFF) << 8);
        Cycles += 7;
    }
    //NOP: 2 cycles
    static void NOP_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        // Do nothing
        Cycles += 2;
    }
    //BIT INSTRUCTIONS
    // Zero Page: 3 cycles
    static void BIT_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Byte addr = cpu.FetchByte(Cycles, bus);
        Byte Value = bus.read(addr);
        cpu.Z = ((Value & cpu.A) == 0) ? 1 : 0;
        cpu.N = (Value & 0b10000000) ? 1 : 0;
        cpu.V = (Value & 0b01000000) ? 1 : 0;
        Cycles += 3;
    }
    // Absolute: 4 cycles
    static void BIT_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Word addr = cpu.FetchWord(Cycles, bus);
        Byte Value = bus.read(addr);
        cpu.Z = ((Value & cpu.A) == 0) ? 1 : 0;
        cpu.N = (Value & 0b10000000) ? 1 : 0;
        cpu.V = (Value & 0b01000000) ? 1 : 0;
        Cycles += 4;
    }

    bool operator==(const InstructionHandlers& other) const
    {
        return false;
    }
};


