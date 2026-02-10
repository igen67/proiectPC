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
    if (pressed) currState[controller] |= mask;
    else currState[controller] &= ~mask;
}

void Input::SetButtons(int controller, uint8_t buttons) {
    if (controller < 0 || controller > 1) return;
    currState[controller] = buttons;
    if (strobe) {
        // while strobe is high, latch immediately
        shiftReg[controller] = ~currState[controller];
        shiftIndex[controller] = 0;
    }
}

uint8_t Input::GetButtons(int controller) const {
    if (controller < 0 || controller > 1) return 0;
    return currState[controller];
}

void Input::WriteStrobe(uint8_t value) {
    bool newStrobe = value & 1;

    // Latch ONLY on 1 -> 0
    if (strobe && !newStrobe) {
        shiftReg[0] = ~currState[0];
        shiftReg[1] = ~currState[1];
        shiftIndex[0] = 0;
        shiftIndex[1] = 0;
    }

    strobe = newStrobe;
}

uint8_t Input::Read(int port) {
    if (port < 0 || port > 1)
        return 0x40;

    uint8_t bit;

    if (strobe) {
        // While strobe high, always return A
        bit = (~currState[port]) & 1;
    } else {
        if (shiftIndex[port] < 8) {
            bit = shiftReg[port] & 1;
            shiftReg[port] >>= 1;
            shiftReg[port] |= 0x80; // pull-up
            shiftIndex[port]++;
        } else {
            bit = 1; // open bus after 8 reads
        }
    }

    return bit | 0x40;
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
#else
    (void)window; // GLFW not available at compile-time: do nothing
#endif
}
