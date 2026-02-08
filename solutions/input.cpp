#include "headers/input.h"
#if defined(__has_include)
#  if __has_include(<GLFW/glfw3.h>)
#    include <GLFW/glfw3.h>
#    define INPUT_HAVE_GLFW 1
#  else
#    define INPUT_HAVE_GLFW 0
#  endif
#else
#  define INPUT_HAVE_GLFW 0
#endif
#include <cstring>
#include <iostream>

Input::Input() {
    currState[0] = currState[1] = 0;
    shiftReg[0] = shiftReg[1] = 0;
    strobe = false;
    shiftIndex[0] = shiftIndex[1] = 0;
}

void Input::SetButton(int controller, Button b, bool pressed) {
    if (controller < 0 || controller > 1) return;
    uint8_t mask = (1u << int(b));
    bool old = (currState[controller] & mask) != 0;
    if (pressed) currState[controller] |= mask;
    else currState[controller] &= ~mask;
    if (old != pressed) {
        const char* names[] = {"A","B","Select","Start","Up","Down","Left","Right"};
        const char* s = (int(b) >= 0 && int(b) < 8) ? names[int(b)] : "?";
        std::cout << "Input: Controller " << controller << " " << s << " " << (pressed ? "pressed" : "released") << std::endl;
    }
}

void Input::SetButtons(int controller, uint8_t buttons) {
    if (controller < 0 || controller > 1) return;
    uint8_t old = currState[controller];
    currState[controller] = buttons;
    if (strobe) {
        // when strobe is high, latch immediately (NES controllers are active-low)
        shiftReg[controller] = static_cast<uint8_t>(~currState[controller]);
        shiftIndex[controller] = 0;
    }
    // Log changes for debugging
    if (old != buttons) {
        std::cout << "Input: SetButtons(controller=" << controller << ", buttons=0x" << std::hex << int(buttons) << std::dec << ")" << std::endl;
    }
}

uint8_t Input::GetButtons(int controller) const {
    if (controller < 0 || controller > 1) return 0;
    return currState[controller];
}

void Input::WriteStrobe(uint8_t value) {
    bool newStrobe = (value & 1) != 0;
    if (newStrobe != strobe) std::cout << "Input: Strobe=" << int(newStrobe) << std::endl;
    if (newStrobe) {
        // When strobe is high, latch current state continuously
        strobe = true;
        // latch inverted (active-low) button states into shift registers
        shiftReg[0] = static_cast<uint8_t>(~currState[0]);
        shiftReg[1] = static_cast<uint8_t>(~currState[1]);
        shiftIndex[0] = shiftIndex[1] = 0;
    } else {
        // On falling edge we start shifting from bit 0
        if (strobe) {
            // falling edge
            shiftIndex[0] = shiftIndex[1] = 0;
        }
        strobe = false;
    }
}

uint8_t Input::Read(int controller) const {
    if (controller < 0 || controller > 1) return 0;
    if (strobe) {
        // while strobe is high, return latched A button state (active-low stored in shiftReg)
        uint8_t v = (shiftReg[controller] & 1) ? 1 : 0;
        std::cout << "Input: Read(controller=" << controller << ", strobe=1) -> " << int(v) << std::endl;
        return v;
    }

    if (shiftIndex[controller] < 8) {
        uint8_t bit = (shiftReg[controller] >> shiftIndex[controller]) & 1;
        std::cout << "Input: Read(controller=" << controller << ", idx=" << shiftIndex[controller] << ", bit=" << int(bit) << ")" << std::endl;
        shiftIndex[controller]++;
        return bit;
    }
    // After 8 reads many controllers return 1
    return 1;
}

// Basic keyboard mapping:
// Z -> A, X -> B, Right Shift -> Select, Enter -> Start
// Arrow keys -> D-Pad
void Input::PollFromGLFW(GLFWwindow* window) {
#if INPUT_HAVE_GLFW
    if (!window) return;
    SetButton(0, BTN_A, glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS);
    SetButton(0, BTN_B, glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS);
    SetButton(0, BTN_SELECT, glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
    SetButton(0, BTN_START, glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS);
    SetButton(0, BTN_UP, glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS);
    SetButton(0, BTN_DOWN, glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS);
    SetButton(0, BTN_LEFT, glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS);
    SetButton(0, BTN_RIGHT, glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS);
    // indicate that a poll occurred (helpful when GUI active)
    static int pollCount = 0;
    if (++pollCount % 60 == 0) std::cout << "Input: PollFromGLFW() - poll #" << pollCount << std::endl;
#else
    (void)window; // GLFW not available at compile-time: do nothing
#endif
}
