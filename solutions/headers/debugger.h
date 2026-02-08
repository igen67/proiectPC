#pragma once
#include <unordered_set>
#include <cstdint>

struct Debugger {
    bool enabled = false;
    bool stepMode = false;
    bool pause = false;

    std::unordered_set<uint16_t> breakpoints;

    void CheckBreakpoint(uint16_t pc) {
        if (breakpoints.count(pc)) {
            pause = true;
        }
    }
};