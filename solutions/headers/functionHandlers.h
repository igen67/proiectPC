#pragma once

#include "cpu.h"
#include "bus.h"
#include "types.h"

//cycles are hell


struct InstructionHandlers
{
    //LDA INSTRUCTIONS
    // Immediate: 2 cycles
    static void LDA_IM_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.A = cpu.FetchByte(Cycles, bus);
        cpu.LDASetStatus();
        Cycles += 0;
    }
    // Zero Page: 3 cycles
    static void LDA_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.A = bus.read(cpu.FetchByte(Cycles, bus));
        cpu.LDASetStatus();
        Cycles += 1;
    }
    // Zero Page,X: 4 cycles
    static void LDA_ZPX_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Byte zp = cpu.FetchByte(Cycles, bus);
        Byte Address = Byte(zp + cpu.X); // force wrap
        cpu.A = bus.read(Address);
        cpu.LDASetStatus();
        Cycles += 2;
    }
    // Absolute: 4 cycles
    static void LDA_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Word Address = cpu.FetchWord(Cycles, bus);
        cpu.A = bus.read(Address);
        cpu.LDASetStatus();
        Cycles += 1;
    }
    // Absolute,X: 4 cycles (+1 if page crossed)
    static void LDA_ABSX_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Word Address = cpu.FetchWord(Cycles, bus);
        bool pageCrossed = ((Address & 0xFF00) != ((Address + cpu.X) & 0xFF00));
        cpu.A = bus.read(Address + cpu.X);
        cpu.LDASetStatus();
        Cycles += 1 + (pageCrossed ? 1 : 0);
    }
    // Absolute,Y: 4 cycles (+1 if page crossed)
    static void LDA_ABSY_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Word Address = cpu.FetchWord(Cycles, bus);
        bool pageCrossed = ((Address & 0xFF00) != ((Address + cpu.Y) & 0xFF00));
        cpu.A = bus.read(Address + cpu.Y);
        cpu.LDASetStatus();
        Cycles += 1 + (pageCrossed ? 1 : 0);
    }
    // Indirect,X: 6 cycles
    static void LDA_INDX_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.A = bus.read(cpu.indirectAddrModeX(Cycles, bus));
        cpu.LDASetStatus();
        Cycles += 0;
    }
    // Indirect,Y: 5 cycles (+1 if page crossed)
    static void LDA_INDY_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Word addr = cpu.indirectAddrModeY(Cycles, bus);
        cpu.A = bus.read(addr);
        cpu.LDASetStatus();
        Cycles += 0;
    }




    //STA INSTRUCTIONS
    // Zero Page: 3 cycles
    static void STA_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Byte Address = cpu.FetchByte(Cycles, bus);
        bus.write(Address, cpu.A);
        Cycles += 1;
    }
    // Zero Page,X: 4 cycles
    static void STA_ZPX_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
Byte zp = cpu.FetchByte(Cycles, bus);
Byte Address = Byte(zp + cpu.X); // force wrap
bus.write(Address, cpu.A);
Cycles += 2;
    }
    // Absolute: 4 cycles
    static void STA_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Word Address = cpu.FetchWord(Cycles, bus);
        bus.write(Address, cpu.A);
        Cycles += 1;
    }
    // Absolute,X: 5 cycles
    static void STA_ABSX_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Word Address = cpu.FetchWord(Cycles, bus) + cpu.X;
        bus.write(Address, cpu.A);
        Cycles += 2;
    }
    // Absolute,Y: 5 cycles
    static void STA_ABSY_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Word Address = cpu.FetchWord(Cycles, bus) + cpu.Y;
        bus.write(Address, cpu.A);
        Cycles += 2;
    }
    // Indirect,X: 6 cycles
    static void STA_INDX_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        bus.write(cpu.indirectAddrModeX(Cycles, bus), cpu.A);
        Cycles += 0;
    }
    // Indirect,Y: 6 cycles
    static void STA_INDY_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        bus.write(cpu.indirectAddrModeY(Cycles, bus, false), cpu.A);
        Cycles += 1;
    }
    //STX INSTRUCTIONS
    // Zero Page: 3 cycles
    static void STX_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Byte Address = cpu.FetchByte(Cycles, bus);
        bus.write(Address, cpu.X);
        Cycles += 1;
    }
    // Zero Page,Y: 4 cycles
    static void STX_ZPY_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Byte zp = cpu.FetchByte(Cycles, bus);
Byte Address = Byte(zp + cpu.Y); // force wrap
        bus.write(Address, cpu.X);
        Cycles += 2;
    }
    // Absolute: 4 cycles
    static void STX_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Word Address = cpu.FetchWord(Cycles, bus);
        bus.write(Address, cpu.X);
        Cycles += 1;
    }
    //STY INSTRUCTIONS
    // Zero Page: 3 cycles
    static void STY_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Byte Address = cpu.FetchByte(Cycles, bus);
        bus.write(Address, cpu.Y);
        Cycles += 1;
    }
    // Zero Page,X: 4 cycles
    static void STY_ZPX_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Byte zp = cpu.FetchByte(Cycles, bus);
        Byte Address = Byte(zp + cpu.X); // force wrap
        bus.write(Address, cpu.Y);
        Cycles += 2;
    }
    // Absolute: 4 cycles
    static void STY_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Word Address = cpu.FetchWord(Cycles, bus);
        bus.write(Address, cpu.Y);
        Cycles += 1;
    }
    //LDX INSTRUCTIONS
    // Immediate: 2 cycles
    static void LDX_IM_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.X = cpu.FetchByte(Cycles, bus);
        cpu.LDXSetStatus();
        Cycles += 0;
    }
    // Zero Page: 3 cycles
    static void LDX_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.X = bus.read(cpu.FetchByte(Cycles, bus));
        cpu.LDXSetStatus();
        Cycles += 1;
    }
    // Zero Page,Y: 4 cycles
    static void LDX_ZPY_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Byte zp = cpu.FetchByte(Cycles, bus);
Byte Address = Byte(zp + cpu.Y); // force wrap
        cpu.X = bus.read(Address);
        cpu.LDXSetStatus();
        Cycles += 2;
    }
    // Absolute: 4 cycles
    static void LDX_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.X = bus.read(cpu.FetchWord(Cycles, bus));
        cpu.LDXSetStatus();
        Cycles += 1;
    }
    // Absolute,Y: 4 cycles (+1 if page crossed)
    static void LDX_ABSY_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Word Address = cpu.FetchWord(Cycles, bus);
        bool pageCrossed = ((Address & 0xFF00) != ((Address + cpu.Y) & 0xFF00));
        cpu.X = bus.read(Address + cpu.Y);
        cpu.LDXSetStatus();
        Cycles += 1 + (pageCrossed ? 1 : 0);
    }
    //LDY INSTRUCTIONS
    // Immediate: 2 cycles
    static void LDY_IM_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.Y = cpu.FetchByte(Cycles, bus);
        cpu.LDYSetStatus();
        Cycles += 0;
    }
    // Zero Page: 3 cycles
    static void LDY_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.Y = bus.read(cpu.FetchByte(Cycles, bus));
        cpu.LDYSetStatus();
        Cycles += 1;
    }
    // Zero Page,X: 4 cycles
    static void LDY_ZPX_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Byte zp = cpu.FetchByte(Cycles, bus);
Byte Address = Byte(zp + cpu.X); // force wrap
        cpu.Y = bus.read(Address);
        cpu.LDYSetStatus();
        Cycles += 2;
    }
    // Absolute: 4 cycles
    static void LDY_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.Y = bus.read(cpu.FetchWord(Cycles, bus));
        cpu.LDYSetStatus();
        Cycles += 1;
    }
    // Absolute,X: 4 cycles (+1 if page crossed)
    static void LDY_ABSX_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Word Address = cpu.FetchWord(Cycles, bus);
        bool pageCrossed = ((Address & 0xFF00) != ((Address + cpu.X) & 0xFF00));
        cpu.Y = bus.read(Address + cpu.X);
        cpu.LDYSetStatus();
        Cycles += 1 + (pageCrossed ? 1 : 0);
    }
    //REGISTER INSTRUCTIONS
    // All implied, 2 cycles
    static void TAX_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.X = cpu.A;
        cpu.SetZN(cpu.X);
        Cycles += 1;
    }
    static void TAY_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.Y = cpu.A;
        cpu.SetZN(cpu.Y);
        Cycles += 1;
    }
    static void TYA_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.A = cpu.Y;
        cpu.SetZN(cpu.A);
        Cycles += 1;
    }
    static void TXA_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.A = cpu.X;
        cpu.SetZN(cpu.A);
        Cycles += 1;
    }
    static void TXS_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        // TXS: 2 cycles
        cpu.SP = cpu.X;
        Cycles += 1;
    }
    static void TSX_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        // TSX: 2 cycles
        cpu.X = cpu.SP;
        cpu.SetZN(cpu.X);

        Cycles += 1;
    }


    //MEMORY MANIP INSTRUCTIONS
    static void DEC_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte Address = cpu.FetchByte(Cycles, bus);
        Byte val = bus.read(Address);
        val -= 1;
        bus.write(Address, val);
        cpu.SetZN(val);
        Cycles += 3;
    }
    static void DEC_ZPX_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte zp = cpu.FetchByte(Cycles, bus);
        Byte Address = Byte(zp + cpu.X); // force wrap
        Byte val = bus.read(Address);
        val -= 1;
        bus.write(Address, val);
        cpu.SetZN(val);
        Cycles += 4;
    }

    static void DEC_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Word Address = cpu.FetchWord(Cycles, bus);
        Byte val = bus.read(Address);
        val -= 1;
        bus.write(Address, val);
        cpu.SetZN(val);
        Cycles += 3;
    }

    static void DEC_ABSX_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Word Address = cpu.FetchWord(Cycles, bus)+cpu.X;
        Byte val = bus.read(Address);
        val -= 1;
        bus.write(Address, val);
        cpu.SetZN(val);
        Cycles += 4;
    }


    static void INC_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Byte Address = cpu.FetchByte(Cycles, bus);
        Byte val = bus.read(Address);
        val += 1;
        bus.write(Address, val);
        cpu.SetZN(val);
        Cycles += 3;
    }
    static void INC_ZPX_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte zp = cpu.FetchByte(Cycles, bus);
        Byte Address = Byte(zp + cpu.X); // force wrap
        Byte val = bus.read(Address);
        val += 1;
        bus.write(Address, val);
        cpu.SetZN(val);
        Cycles += 4;
    }

    static void INC_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Word Address = cpu.FetchWord(Cycles, bus);
        Byte val = bus.read(Address);
        val += 1;
        bus.write(Address, val);
        cpu.SetZN(val);
        Cycles += 3;
    }

    static void INC_ABSX_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Word Address = cpu.FetchWord(Cycles, bus)+cpu.X;
        Byte val = bus.read(Address);
        val += 1;
        bus.write(Address, val);
        cpu.SetZN(val);
        Cycles += 4;
    }














    //ALGEBRA AND LOGIC INSTRUCIONS


    static void SBC_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        cpu.SBCSetStatus(bus.read(cpu.FetchByte(Cycles, bus)));
        Cycles += 1;
    }
    static void SBC_IM_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.SBCSetStatus(cpu.FetchByte(Cycles, bus));
        Cycles += 0;
    }
    static void SBC_ZPX_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        Byte zp = cpu.FetchByte(Cycles, bus);
        Byte Address = Byte(zp + cpu.X); // force wrap
        cpu.SBCSetStatus(bus.read(Address));
        Cycles += 2;
    }
    static void SBC_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.SBCSetStatus(bus.read(cpu.FetchWord(Cycles, bus)));
        Cycles += 1;
    }
    static void SBC_ABSX_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        Word Address = cpu.FetchWord(Cycles, bus);
        bool pageCrossed = ((Address & 0xFF00) != ((Address + cpu.X) & 0xFF00));
        cpu.SBCSetStatus(bus.read(Address + cpu.X));
        Cycles += 1 + (pageCrossed ? 1 : 0);
    }
    static void SBC_ABSY_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        Word Address = cpu.FetchWord(Cycles, bus);
        bool pageCrossed = ((Address & 0xFF00) != ((Address + cpu.Y) & 0xFF00));
        cpu.SBCSetStatus(bus.read(Address + cpu.Y));
        Cycles += 1 + (pageCrossed ? 1 : 0);
    }
    static void SBC_INDX_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.SBCSetStatus(bus.read(cpu.indirectAddrModeX(Cycles, bus)));
        Cycles += 0;
    }
    static void SBC_INDY_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.SBCSetStatus(bus.read(cpu.indirectAddrModeY(Cycles, bus)));
        Cycles += 0;
    }


    static void AND_IM_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.A = cpu.A & cpu.FetchByte(Cycles, bus);
        cpu.AndSetStatus();
        Cycles += 0;
    }
    static void AND_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A & bus.read(cpu.FetchByte(Cycles, bus));
        cpu.AndSetStatus();
        Cycles += 1;
    }
    static void AND_ZPX_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        Byte zp = cpu.FetchByte(Cycles, bus);
        Byte Address = Byte(zp + cpu.X); // force wrap
        cpu.A = cpu.A & bus.read(Address);
        cpu.AndSetStatus();
        Cycles += 2;
    }
    static void AND_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A & bus.read(cpu.FetchWord(Cycles, bus));
        cpu.AndSetStatus();
        Cycles += 1;
    }
    static void AND_ABSX_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        Word Address = cpu.FetchWord(Cycles, bus);
        bool pageCrossed = ((Address & 0xFF00) != ((Address + cpu.X) & 0xFF00));
        cpu.A = cpu.A & bus.read(Address + cpu.X);
        cpu.AndSetStatus();
        Cycles += 1 + (pageCrossed ? 1 : 0);
    }
    static void AND_ABSY_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        Word Address = cpu.FetchWord(Cycles, bus);
        bool pageCrossed = ((Address & 0xFF00) != ((Address + cpu.Y) & 0xFF00));
        cpu.A = cpu.A & bus.read(Address + cpu.Y);
        cpu.AndSetStatus();
        Cycles += 1 + (pageCrossed ? 1 : 0);
    }
    static void AND_INDX_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A & bus.read(cpu.indirectAddrModeX(Cycles, bus));
        cpu.AndSetStatus();
        Cycles += 0;
    }
    static void AND_INDY_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A & bus.read(cpu.indirectAddrModeY(Cycles, bus));
        cpu.AndSetStatus();
        Cycles += 0;
    }

    static void ORA_IM_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A | cpu.FetchByte(Cycles, bus);
        cpu.LDASetStatus();
        Cycles += 0;
    }
    static void ORA_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A | bus.read(cpu.FetchByte(Cycles, bus));
        cpu.LDASetStatus();
        Cycles += 1;
    }
    static void ORA_ZPX_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        Byte zp = cpu.FetchByte(Cycles, bus);
        Byte Address = Byte(zp + cpu.X); // force wrap
        cpu.A = cpu.A | bus.read(Address);
        cpu.AndSetStatus();
        Cycles += 2;
    }
    static void ORA_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A | bus.read(cpu.FetchWord(Cycles, bus));
        cpu.AndSetStatus();
        Cycles += 1;
    }
    static void ORA_ABSX_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        Word Address = cpu.FetchWord(Cycles, bus);
        bool pageCrossed = ((Address & 0xFF00) != ((Address + cpu.X) & 0xFF00));
        cpu.A = cpu.A | bus.read(Address + cpu.X);
        cpu.AndSetStatus();
        Cycles += 1 + (pageCrossed ? 1 : 0);
    }
static void ORA_ABSY_Handler(CPU& cpu, u32& Cycles, Bus& bus)
{
    Word base = cpu.FetchWord(Cycles, bus);
    Word addr = base + cpu.Y;

    bool pageCrossed = (base & 0xFF00) != (addr & 0xFF00);

    cpu.A |= bus.read(addr);
    cpu.AndSetStatus();

    Cycles += 1 + (pageCrossed ? 1 : 0);
}
    static void ORA_INDX_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A | bus.read(cpu.indirectAddrModeX(Cycles, bus));
        cpu.AndSetStatus();
        Cycles += 0;
    }
    static void ORA_INDY_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A | bus.read(cpu.indirectAddrModeY(Cycles, bus));
        cpu.AndSetStatus();
        Cycles += 1;
    }

    static void EOR_IM_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A ^ cpu.FetchByte(Cycles, bus);
        cpu.LDASetStatus();
        Cycles += 0;
    }
    static void EOR_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A ^ bus.read(cpu.FetchByte(Cycles, bus));
        cpu.LDASetStatus();
        Cycles += 1;
    }
    static void EOR_ZPX_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        Byte zp = cpu.FetchByte(Cycles, bus);
Byte Address = Byte(zp + cpu.X); // force wrap
        cpu.A = cpu.A ^ bus.read(Address);
        cpu.AndSetStatus();
        Cycles += 2;
    }
    static void EOR_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A ^ bus.read(cpu.FetchWord(Cycles, bus));
        cpu.AndSetStatus();
        Cycles += 1;
    }
    static void EOR_ABSX_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        Word Address = cpu.FetchWord(Cycles, bus);
        bool pageCrossed = ((Address & 0xFF00) != ((Address + cpu.X) & 0xFF00));
        cpu.A = cpu.A ^ bus.read(Address + cpu.X);
        cpu.AndSetStatus();
        Cycles += 1 + (pageCrossed ? 1 : 0);
    }
    static void EOR_ABSY_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        Word Address = cpu.FetchWord(Cycles, bus);
        bool pageCrossed = ((Address & 0xFF00) != ((Address + cpu.Y) & 0xFF00));
        cpu.A = cpu.A ^ bus.read(Address + cpu.Y);
        cpu.AndSetStatus();
        Cycles += 1 + (pageCrossed ? 1 : 0);
    }
    static void EOR_INDX_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A ^ bus.read(cpu.indirectAddrModeX(Cycles, bus));
        cpu.AndSetStatus();
        Cycles += 0;
    }
    static void EOR_INDY_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.A = cpu.A ^ bus.read(cpu.indirectAddrModeY(Cycles, bus));
        cpu.AndSetStatus();
        Cycles += 0;
    }


    static void CMP_IM_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte Value = cpu.FetchByte(Cycles, bus);
        Byte Temp = cpu.A - Value;
        cpu.SetFlag(CPU::FLAG_C, cpu.A >= Value);
        cpu.SetZN(Temp);
        Cycles += 0;
        if (g_verboseCpu) {
            printf("CMP: A = 0x%02X, Value = 0x%02X, Z = %d, C = %d, N = %d\n",
                cpu.A, Value,
                cpu.GetFlag(CPU::FLAG_Z),
                cpu.GetFlag(CPU::FLAG_C),
                cpu.GetFlag(CPU::FLAG_N));
        }
    }
    static void CMP_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte Value = bus.read(cpu.FetchByte(Cycles, bus));
        Byte Temp = cpu.A - Value;
        cpu.SetFlag(CPU::FLAG_C, cpu.A >= Value);
        cpu.SetZN(Temp);
        Cycles += 1;
        if (g_verboseCpu) {
            printf("CMP: A = 0x%02X, Value = 0x%02X, Z = %d, C = %d, N = %d\n",
                cpu.A, Value,
                cpu.GetFlag(CPU::FLAG_Z),
                cpu.GetFlag(CPU::FLAG_C),
                cpu.GetFlag(CPU::FLAG_N));
        }
    }
    static void CMP_ZPX_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte zp = cpu.FetchByte(Cycles, bus);
Byte Address = Byte(zp + cpu.X); // force wrap
        Byte Value = bus.read(Address);
        Byte Temp = cpu.A - Value;
        cpu.SetFlag(CPU::FLAG_C, cpu.A >= Value);
        cpu.SetZN(Temp);
        Cycles += 2;
        if (g_verboseCpu) {
            printf("CMP: A = 0x%02X, Value = 0x%02X, Z = %d, C = %d, N = %d\n",
                cpu.A, Value,
                cpu.GetFlag(CPU::FLAG_Z),
                cpu.GetFlag(CPU::FLAG_C),
                cpu.GetFlag(CPU::FLAG_N));
        }
    }
    static void CMP_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte Value = bus.read(cpu.FetchWord(Cycles, bus));
        Byte Temp = cpu.A - Value;
        cpu.SetFlag(CPU::FLAG_C, cpu.A >= Value);
        cpu.SetZN(Temp);
        Cycles += 1;
        if (g_verboseCpu) {
            printf("CMP: A = 0x%02X, Value = 0x%02X, Z = %d, C = %d, N = %d\n",
                cpu.A, Value,
                cpu.GetFlag(CPU::FLAG_Z),
                cpu.GetFlag(CPU::FLAG_C),
                cpu.GetFlag(CPU::FLAG_N));
        }
    }
    static void CMP_ABSX_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Word Address = cpu.FetchWord(Cycles, bus);
        bool pageCrossed = ((Address & 0xFF00) != ((Address + cpu.X) & 0xFF00));
        Byte Value = bus.read(Address + cpu.X);
        Byte Temp = cpu.A - Value;
        cpu.SetFlag(CPU::FLAG_C, cpu.A >= Value);
        cpu.SetZN(Temp);
        Cycles += 1 + (pageCrossed ? 1 : 0);
        if (g_verboseCpu) {
            printf("CMP: A = 0x%02X, Value = 0x%02X, Z = %d, C = %d, N = %d\n",
                cpu.A, Value,
                cpu.GetFlag(CPU::FLAG_Z),
                cpu.GetFlag(CPU::FLAG_C),
                cpu.GetFlag(CPU::FLAG_N));
        }
    }
    static void CMP_ABSY_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Word Address = cpu.FetchWord(Cycles, bus);
        bool pageCrossed = ((Address & 0xFF00) != ((Address + cpu.Y) & 0xFF00));
        Byte Value = bus.read(Address + cpu.Y);
        Byte Temp = cpu.A - Value;
        cpu.SetFlag(CPU::FLAG_C, cpu.A >= Value);
        cpu.SetZN(Temp);
        Cycles += 1 + (pageCrossed ? 1 : 0);
        if (g_verboseCpu) {
            printf("CMP: A = 0x%02X, Value = 0x%02X, Z = %d, C = %d, N = %d\n",
                cpu.A, Value,
                cpu.GetFlag(CPU::FLAG_Z),
                cpu.GetFlag(CPU::FLAG_C),
                cpu.GetFlag(CPU::FLAG_N));
        }
    }
    static void CMP_INDX_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte Value = bus.read(cpu.indirectAddrModeX(Cycles, bus));
        Byte Temp = cpu.A - Value;
        cpu.SetFlag(CPU::FLAG_C, cpu.A >= Value);
        cpu.SetZN(Temp);
        Cycles += 0;
        if (g_verboseCpu) {
            printf("CMP: A = 0x%02X, Value = 0x%02X, Z = %d, C = %d, N = %d\n",
                cpu.A, Value,
                cpu.GetFlag(CPU::FLAG_Z),
                cpu.GetFlag(CPU::FLAG_C),
                cpu.GetFlag(CPU::FLAG_N));
        }
    }
    static void CMP_INDY_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte Value = bus.read(cpu.indirectAddrModeY(Cycles, bus));
        Byte Temp = cpu.A - Value;
        cpu.SetFlag(CPU::FLAG_C, cpu.A >= Value);
        cpu.SetZN(Temp);
        Cycles += 0;
        if (g_verboseCpu) {
            printf("CMP: A = 0x%02X, Value = 0x%02X, Z = %d, C = %d, N = %d\n",
                cpu.A, Value,
                cpu.GetFlag(CPU::FLAG_Z),
                cpu.GetFlag(CPU::FLAG_C),
                cpu.GetFlag(CPU::FLAG_N));
        }
    }

    static void CPX_IM_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        Byte Value = cpu.FetchByte(Cycles, bus);
        Byte Temp = cpu.X - Value;
        cpu.SetFlag(CPU::FLAG_C, cpu.X >= Value);
        cpu.SetZN(Temp);
        Cycles += 0;
    }

    static void CPX_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        Byte Value = bus.read(cpu.FetchByte(Cycles, bus));
        Byte Temp = cpu.X - Value;
        cpu.SetFlag(CPU::FLAG_C, cpu.X >= Value);
        cpu.SetZN(Temp);
        Cycles += 1;
    }

    static void CPX_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        Byte Value = bus.read(cpu.FetchWord(Cycles, bus));
        Byte Temp = cpu.X - Value;
        cpu.SetFlag(CPU::FLAG_C, cpu.X >= Value);
        cpu.SetZN(Temp);
        Cycles += 1;
    }

    static void CPY_IM_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        Byte Value = cpu.FetchByte(Cycles, bus);
        Byte Temp = cpu.Y - Value;
        cpu.SetFlag(CPU::FLAG_C, cpu.Y >= Value);
        cpu.SetZN(Temp);
        Cycles += 0;
    }

    static void CPY_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        Byte Value = bus.read(cpu.FetchByte(Cycles, bus));
        Byte Temp = cpu.Y - Value;
        cpu.SetFlag(CPU::FLAG_C, cpu.Y >= Value);
        cpu.SetZN(Temp);
        Cycles += 1;
    }

    static void CPY_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        Byte Value = bus.read(cpu.FetchWord(Cycles, bus));
        Byte Temp = cpu.Y - Value;
        cpu.SetFlag(CPU::FLAG_C, cpu.Y >= Value);
        cpu.SetZN(Temp);
        Cycles += 1;
    }




    static void ADC_IM_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.ADCSetStatus(cpu.FetchByte(Cycles, bus));
        Cycles += 0;
    }

    static void ADC_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.ADCSetStatus(bus.read(cpu.FetchByte(Cycles, bus)));
        Cycles += 1;
    }
    static void ADC_ZPX_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        Byte zp = cpu.FetchByte(Cycles, bus);
Byte Address = Byte(zp + cpu.X); // force wrap
        cpu.ADCSetStatus(bus.read(Address));
        Cycles += 2;
    }
    static void ADC_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.ADCSetStatus(bus.read(cpu.FetchWord(Cycles, bus)));
        Cycles += 1;
    }
    static void ADC_ABSX_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        Word Address = cpu.FetchWord(Cycles, bus);
        bool pageCrossed = ((Address & 0xFF00) != ((Address + cpu.X) & 0xFF00));
        cpu.ADCSetStatus(bus.read(Address + cpu.X));
        Cycles += 1 + (pageCrossed ? 1 : 0);
    }
    static void ADC_ABSY_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        Word Address = cpu.FetchWord(Cycles, bus);
        bool pageCrossed = ((Address & 0xFF00) != ((Address + cpu.Y) & 0xFF00));
        cpu.ADCSetStatus(bus.read(Address + cpu.Y));
        Cycles += 1 + (pageCrossed ? 1 : 0);

    }
    static void ADC_INDX_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.ADCSetStatus(bus.read(cpu.indirectAddrModeX(Cycles, bus)));
        Cycles += 0;
    }
    static void ADC_INDY_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.ADCSetStatus(bus.read(cpu.indirectAddrModeY(Cycles, bus)));
        Cycles += 0; // page-cross handled in addr mode
    }

    //SHIFT AND ROTATE
    static void ASL_A_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        cpu.ASL(bus, cpu.A);
        Cycles += 1;
    }

    static void ASL_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte addr = cpu.FetchByte(Cycles, bus);
        Byte val = bus.read(addr);
        cpu.ASL(bus, val);
        bus.write(addr, val);
        Cycles += 3;
    }

    static void ASL_ZPX_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte zp = cpu.FetchByte(Cycles, bus);
Byte Address = Byte(zp + cpu.X); // force wrap
        Byte val = bus.read(Address);
        cpu.ASL(bus, val);
        bus.write(Address, val);
        Cycles += 4;
    }

    static void ASL_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Word addr = cpu.FetchWord(Cycles, bus);
        Byte val = bus.read(addr);
        cpu.ASL(bus, val);
        bus.write(addr, val);
        Cycles += 3;
    }

static void ASL_ABSX_Handler(CPU& cpu, u32& Cycles, Bus& bus)
{
    Word base = cpu.FetchWord(Cycles, bus);
    Word addr = base + cpu.X;

    Byte val = bus.read(addr);
    cpu.ASL(bus, val);
    bus.write(addr, val);

    Cycles += 4;
}

    static void LSR_A_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        cpu.LSR(bus, cpu.A);
        Cycles += 1;
    }

    static void LSR_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte addr = cpu.FetchByte(Cycles, bus);
        Byte val = bus.read(addr);
        cpu.LSR(bus, val);
        bus.write(addr, val);
        Cycles += 3;
    }

    static void LSR_ZPX_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte zp = cpu.FetchByte(Cycles, bus);
Byte Address = Byte(zp + cpu.X); // force wrap
        Byte val = bus.read(Address);
        cpu.LSR(bus, val);
        bus.write(Address, val);
        Cycles += 4;
    }

    static void LSR_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Word addr = cpu.FetchWord(Cycles, bus);
        Byte val = bus.read(addr);
        cpu.LSR(bus, val);
        bus.write(addr, val);
        Cycles += 3;
    }

    static void LSR_ABSX_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Word addr = cpu.FetchWord(Cycles, bus)+cpu.X;
        Byte val = bus.read(addr);
        cpu.LSR(bus, val);
        bus.write(addr, val);
        Cycles += 4;
    }

    static void ROL_A_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        cpu.ROL(bus, cpu.A);
        Cycles += 1;
    }

    static void ROL_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte addr = cpu.FetchByte(Cycles, bus);
        Byte val = bus.read(addr);
        cpu.ROL(bus, val);
        bus.write(addr, val);
        Cycles += 3;
    }

    static void ROL_ZPX_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte zp = cpu.FetchByte(Cycles, bus);
Byte Address = Byte(zp + cpu.X); // force wrap
        Byte val = bus.read(Address);
        cpu.ROL(bus, val);
        bus.write(Address, val);
        Cycles += 4;
    }

    static void ROL_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Word addr = cpu.FetchWord(Cycles, bus);
        Byte val = bus.read(addr);
        cpu.ROL(bus, val);
        bus.write(addr, val);
        Cycles += 3;
    }

    static void ROL_ABSX_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Word addr = cpu.FetchWord(Cycles, bus)+cpu.X;
        Byte val = bus.read(addr);
        cpu.ROL(bus, val);
        bus.write(addr, val);
        Cycles += 4;
    }

    static void ROR_A_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        cpu.ROR(bus, cpu.A);
        Cycles += 1;
    }

    static void ROR_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte addr = cpu.FetchByte(Cycles, bus);
        Byte val = bus.read(addr);
        cpu.ROR(bus, val);
        bus.write(addr, val);
        Cycles += 3;
    }

    static void ROR_ZPX_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Byte zp = cpu.FetchByte(Cycles, bus);
Byte Address = Byte(zp + cpu.X); // force wrap
        Byte val = bus.read(Address);
        cpu.ROR(bus, val);
        bus.write(Address, val);
        Cycles += 4;
    }

    static void ROR_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Word addr = cpu.FetchWord(Cycles, bus);
        Byte val = bus.read(addr);
        cpu.ROR(bus, val);
        bus.write(addr, val);
        Cycles += 3;
    }

    static void ROR_ABSX_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        Word addr = cpu.FetchWord(Cycles, bus)+cpu.X;
        Byte val = bus.read(addr);
        cpu.ROR(bus, val);
        bus.write(addr, val);
        Cycles += 4;
    }





    //REGISTER MANIP
    static void DEX_Handler(CPU& cpu, u32& Cycles, Bus& bus) 
    {
        cpu.X -= 0x01;
        cpu.SetZN(cpu.X);
        Cycles += 1;
    }

       static void INX_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.X += 1;
        cpu.SetZN(cpu.X);

        Cycles += 1;
    }
        static void DEY_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.Y -= 0x01;
            cpu.SetZN(cpu.Y);
            Cycles += 1;
    }
    static void INY_Handler(CPU& cpu, u32& Cycles, Bus& bus)
    {
        cpu.Y += 1;
            cpu.SetZN(cpu.Y);
            Cycles += 1;
    }





    //BRANCH INSTRUCTIONS
    static void BCC_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.ExecuteBranch(Cycles, bus, !cpu.GetFlag(CPU::FLAG_C)); // Handles cycles internally
    }

    static void BCS_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.ExecuteBranch(Cycles, bus, cpu.GetFlag(CPU::FLAG_C));
    }

    static void BEQ_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.ExecuteBranch(Cycles, bus, cpu.GetFlag(CPU::FLAG_Z));
    }

    static void BMI_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.ExecuteBranch(Cycles, bus, cpu.GetFlag(CPU::FLAG_N));
    }

    static void BNE_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.ExecuteBranch(Cycles, bus, !cpu.GetFlag(CPU::FLAG_Z));
    }

    static void BPL_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.ExecuteBranch(Cycles, bus, !cpu.GetFlag(CPU::FLAG_N));
    }

    static void BVC_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.ExecuteBranch(Cycles, bus, !cpu.GetFlag(CPU::FLAG_V));
    }

    static void BVS_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.ExecuteBranch(Cycles, bus, cpu.GetFlag(CPU::FLAG_V));
    }
    //STACK INSTRUCTIONS
    // PHA: 3 cycles
    static void PHA_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        // Push A onto the stack (stack is on page 1, wraps via modifySP)
        bus.write(0x0100 | cpu.SP, cpu.A);
        cpu.SP--;
        Cycles += 2;
    }
    // PLA: 4 cycles
    static void PLA_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.SP++;
        cpu.A = bus.read(0x0100 | cpu.SP);
        cpu.SetZN(cpu.A);
        Cycles += 3;
    }
    // PHP: 3 cycles
    static void PHP_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Byte status = cpu.GetStatus(true);
        bus.write(0x0100 | cpu.SP, status);
        cpu.SP--;
        Cycles += 2;
    }
    // PLP: 4 cycles
    static void PLP_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.SP++;
        cpu.modifySP();
        Byte status = bus.read(0x0100 | cpu.SP);
        cpu.SetStatusFromStack(status);
        Cycles += 3;
    }
    //FLAG INSTRUCTIONS
    // All implied, 2 cycles
    static void CLC_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.SetFlag(CPU::FLAG_C, false);
        Cycles += 1;
    }
    static void CLD_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.SetFlag(CPU::FLAG_D, false);
        Cycles += 1;
    }
    static void CLI_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.SetFlag(CPU::FLAG_I, false);
        Cycles += 1;
    }
    static void CLV_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.SetFlag(CPU::FLAG_V, false);
        Cycles += 1;
    }
    static void SEC_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.SetFlag(CPU::FLAG_C, true);
        Cycles += 1;
    }
    static void SED_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.SetFlag(CPU::FLAG_D, true);
        Cycles += 1;
    }
    static void SEI_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.SetFlag(CPU::FLAG_I, true);
        Cycles += 1;
    }
    //JUMP INSTRUCTIONS
    // JMP Absolute: 3 cycles
    static void JMP_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.PC = cpu.FetchWord(Cycles, bus);
        // debug: printf("JMP to 0x%X\n", cpu.PC);
        Cycles += 0;
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
        Cycles += 2;
    }
    // JSR: 6 cycles
    static void JSR_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Word returnAddress = static_cast<Word>(cpu.PC + 1);
        // push high then low bytes
        bus.write(0x0100 | cpu.SP, (returnAddress >> 8) & 0xFF);
        cpu.SP--;
        bus.write(0x0100 | cpu.SP, returnAddress & 0xFF);
        cpu.SP--;
        cpu.PC = cpu.FetchWord(Cycles, bus);
        Cycles += 3;
    }
    // RTS: 6 cycles
    static void RTS_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        cpu.SP++;
        Byte lo = bus.read(0x0100 | cpu.SP);
        cpu.SP++;
        Byte hi = bus.read(0x0100 | cpu.SP);
        cpu.PC = static_cast<Word>((hi << 8) | lo);
        cpu.PC++;

        Cycles += 5;
    }
    // RTI: 6 cycles
static void RTI_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
    // Pull status
    cpu.SP++;
    Byte status = bus.read(0x0100 | cpu.SP);
    cpu.SetStatusFromStack(status);

    // Pull PC low
    cpu.SP++;
    Byte lo = bus.read(0x0100 | cpu.SP);

    // Pull PC high
    cpu.SP++;
    Byte hi = bus.read(0x0100 | cpu.SP);

    cpu.PC = (hi << 8) | lo;

    Cycles += 5;
}
    // BRK: 7 cycles
    static void BRK_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Word returnAddress = cpu.PC + 2;
        bus.write(0x0100 | cpu.SP, (returnAddress >> 8) & 0xFF);
        cpu.SP--;
        bus.write(0x0100 | cpu.SP, returnAddress & 0xFF);
        // push status with B flag set
        cpu.SP--;
        Byte status = cpu.GetStatus(true);
        bus.write(0x0100 | cpu.SP, status);
        cpu.SP--;
        cpu.SetFlag(CPU::FLAG_I, true);
        cpu.PC = bus.read(0xFFFE) | (bus.read(0xFFFF) << 8);
        Cycles += 6;
    }
    //NOP: 2 cycles
    static void NOP_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        // Do nothing
        Cycles += 1;
    }
    static void NOP_ABSX_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        // Do nothing, but account for extra cycle
        Word Address = cpu.FetchWord(Cycles, bus);
        bool pageCrossed = ((Address & 0xFF00) != ((Address + cpu.X) & 0xFF00));
        Cycles += 1 + (pageCrossed ? 1 : 0);
    }
    static void NOP_ABSY_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
    Word Address = cpu.FetchWord(Cycles, bus);
    bool pageCrossed = ((Address & 0xFF00) != ((Address + cpu.Y) & 0xFF00));
    Cycles += 1 + (pageCrossed ? 1 : 0);
}
    //BIT INSTRUCTIONS
    // Zero Page: 3 cycles
    static void BIT_ZP_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Byte addr = cpu.FetchByte(Cycles, bus);
        Byte Value = bus.read(addr);
        cpu.SetFlag(CPU::FLAG_Z, (Value & cpu.A) == 0);
        cpu.SetFlag(CPU::FLAG_N, (Value & 0x80) != 0);
        cpu.SetFlag(CPU::FLAG_V, (Value & 0x40) != 0);
        Cycles += 1;
    }
    // Absolute: 4 cycles
    static void BIT_ABS_Handler(CPU& cpu, u32& Cycles, Bus& bus) {
        Word addr = cpu.FetchWord(Cycles, bus);
        Byte Value = bus.read(addr);
        cpu.SetFlag(CPU::FLAG_Z, (Value & cpu.A) == 0);
        cpu.SetFlag(CPU::FLAG_N, (Value & 0x80) != 0);
        cpu.SetFlag(CPU::FLAG_V, (Value & 0x40) != 0);
        Cycles += 1;
    }

    bool operator==(const InstructionHandlers& other) const
    {
        return false;
    }
};


