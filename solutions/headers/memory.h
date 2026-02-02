#pragma once
#include "libraries.h"
#include "types.h"
struct Mem {

 

    static constexpr u32 MAX_MEM = 1024 * 64;
    Byte Data[MAX_MEM];

    void Initialise();

    Byte operator[](u32 Address) const;

    Byte& operator[](u32 Address);

    void WriteWord(u32& Cycle, Word DataToWrite, u32 Adress);
    void LoadMachineCodeFromFile(const std::string& filename);


};