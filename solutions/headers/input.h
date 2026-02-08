// Basic NES controller input handling
#pragma once
#include <cstdint>
struct GLFWwindow;

class Input {
public:
    Input();

    enum Button {
        BTN_A = 0,
        BTN_B = 1,
        BTN_SELECT = 2,
        BTN_START = 3,
        BTN_UP = 4,
        BTN_DOWN = 5,
        BTN_LEFT = 6,
        BTN_RIGHT = 7,
    };

    // Set raw button state for controller (0 or 1)
    void SetButton(int controller, Button b, bool pressed);
    // Set all buttons at once (bits as A,B,Select,Start,Up,Down,Left,Right)
    void SetButtons(int controller, uint8_t buttons);
    // Read back current latched button bits
    uint8_t GetButtons(int controller) const;

    // Write to strobe register (0x4016) from CPU
    void WriteStrobe(uint8_t value);

    // Read next bit from controller 0 or 1 (0x4016 / 0x4017)
    uint8_t Read(int controller) const;

    // Poll keyboard via GLFW and update internal current state
    void PollFromGLFW(GLFWwindow* window);

private:
    uint8_t currState[2];    // current live button state bits
    uint8_t shiftReg[2];     // latched shift register used for reads
    bool strobe;
    mutable int shiftIndex[2];
};
