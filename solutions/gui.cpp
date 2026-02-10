#include "gui.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "headers/ppu.h"
#include "headers/cpu.h"
#include <GLFW/glfw3.h> 
#include <GL/gl.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>

// Simple memory hex viewer helper
static void DrawMemoryView(Bus& bus, uint32_t baseAddr, int rows = 16, int cols = 16) {
    ImGui::Begin("Memory");
    uint32_t addr = baseAddr;
    for (int r = 0; r < rows; ++r) {
        ImGui::Text("%04X:", addr);
        ImGui::SameLine();
        ImGui::BeginGroup();
        for (int c = 0; c < cols; ++c) {
            uint8_t v = bus.read(static_cast<uint16_t>(addr));
            ImGui::Text("%02X", v);
            ImGui::SameLine();
            addr++;
        }
        ImGui::EndGroup();
        ImGui::NewLine();
    }
    ImGui::End();
}

void RunGUI(CPU& cpu, Bus& bus) {
    // Setup GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "proiectPC", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        return;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    // PPU + texture for pattern table/frame view
    PPU ppu(bus);
    //ppu.Reset();
    bus.AttachPPU(&ppu);
    // Attach CPU ptr to bus so mappers can request IRQs
    bus.AttachCPU(&cpu);
    unsigned int ppuTex = 0;
    unsigned int patternTex = 0;
    std::vector<uint32_t> ppuPixels;
    int ppuW = 0, ppuH = 0;

    // Create a texture for PPU frame display
    glGenTextures(1, &ppuTex);
    glBindTexture(GL_TEXTURE_2D, ppuTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // Initialize with empty 256x240
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 240, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Create a texture for pattern table display (128x128)
    glGenTextures(1, &patternTex);
    glBindTexture(GL_TEXTURE_2D, patternTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    bool show_demo_window = false;
    bool running = true; // auto-start to measure emulation speed
    bool liveRender = true; // Enable live per-frame updates from PPU
    int patternPaletteGroup = 0; // palette group (0..3) used for pattern table viewer
    u32 cycles = 0;
    char filePath[512] = "";
    uint32_t memBase = 0x0000;

    // Emulation speed measurement
    const double NES_CPU_FREQ = 1789773.0; // NTSC 6502 cycles/sec
    auto measureStart = std::chrono::steady_clock::now();
    uint64_t cyclesAtLastMeasure = 0;
    uint64_t accumCycles = 0;
    double cyclesPerSec = 0.0;
    // Smoothed EMA of cycles/sec for stable UI readout
    double smoothedCyclesPerSec = 0.0;
    const double cyclesPerSecEMAalpha = 0.25;
    auto lastConsolePrint = std::chrono::steady_clock::now();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        // Map keyboard to NES controller and update bus controller state
        uint8_t joy = 0;
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) joy |= (1 << 0); // A
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) joy |= (1 << 1); // B
        if (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) joy |= (1 << 2); // Select
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) joy |= (1 << 3); // Start
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) joy |= (1 << 4); // Up
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) joy |= (1 << 5); // Down
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) joy |= (1 << 6); // Left
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) joy |= (1 << 7); // Right
        bus.SetControllerButtons(joy);

        if (running) {


            // Run enough cycles for one NES frame (about 29780 cycles)
            uint32_t targetCycles = 29780;
            uint32_t executed = 0;
            while (executed < targetCycles) {
                u32 before = cycles;
                cpu.Execute(cycles, bus);
                executed += (cycles - before);
            }

        }

      
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Controls");
        ImGui::InputText("ROM Path", filePath, sizeof(filePath));
        if (ImGui::Button("Load PRG (iNES)")) {
            if (!bus.LoadPRGFromFile(filePath)) {
                std::cerr << "Falling back to RAM load for: " << filePath << std::endl;
                bus.ram.LoadMachineCodeFromFile(filePath);
            }
            cpu.Reset(bus);
        }
        ImGui::SameLine();
        if (ImGui::Button("Load CHR (raw)")) {
            // Quick raw CHR loader for testing pattern tables
            std::ifstream f(filePath, std::ios::binary);
            if (!f) {
                std::cerr << "Failed to open CHR file: " << filePath << std::endl;
            } else {
                f.seekg(0, std::ios::end);
                std::streamsize size = f.tellg();
                f.seekg(0, std::ios::beg);
                bus.chrRom.resize(size);
                f.read(reinterpret_cast<char*>(bus.chrRom.data()), size);
                std::cout << "Loaded raw CHR file: " << size << " bytes\n";
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset CPU")) {
            cpu.Reset(bus);
        }
        ImGui::SameLine();

        ImGui::SameLine();
        if (ImGui::Button(running ? "Pause" : "Run")) {
            running = !running;
        }
        ImGui::SameLine();

        ImGui::SameLine();
        ImGui::Checkbox("Live Render", &liveRender);
        ImGui::SameLine();
        ImGui::Checkbox("Show ImGui Demo", &show_demo_window);

        ImGui::Text("Cycles last step: %u", cycles);

        // Update emulation speed sampler
        auto now = std::chrono::steady_clock::now();
        if (running) {
          
            uint64_t deltaCycles = cycles - cyclesAtLastMeasure;
            cyclesAtLastMeasure = cycles;
            accumCycles += deltaCycles;
        }
        double elapsed = std::chrono::duration<double>(now - measureStart).count();
        if (elapsed >= 0.5) {
            double measured = double(accumCycles) / elapsed;
            // Update EMA smoothed value
            if (smoothedCyclesPerSec <= 0.0) smoothedCyclesPerSec = measured;
            else smoothedCyclesPerSec = cyclesPerSecEMAalpha * measured + (1.0 - cyclesPerSecEMAalpha) * smoothedCyclesPerSec;

            cyclesPerSec = measured;
            accumCycles = 0;
            measureStart = now;
            // Console print no more than once per second
            if (std::chrono::duration<double>(now - lastConsolePrint).count() >= 1.0) {
                //printf("EMU (measured): %.0f cycles/s (%.1f%%) FPS: %.1f -- EMA: %.0f\n",
                       //cyclesPerSec, 100.0 * (cyclesPerSec / NES_CPU_FREQ), io.Framerate, smoothedCyclesPerSec);
                lastConsolePrint = now;
            }
        }

        ImGui::Text("Host FPS: %.1f, Emu speed: %.1f%% (%.0f cycles/s)", io.Framerate, 100.0 * (smoothedCyclesPerSec / NES_CPU_FREQ), smoothedCyclesPerSec);
        ImGui::InputScalar("Mem Base", ImGuiDataType_U32, &memBase, NULL, NULL);
        // Show controller state for debugging
        uint8_t buttons = bus.input.GetButtons(0);
        ImGui::Text("Controller0: 0x%02X", buttons);
        ImGui::SameLine(); ImGui::Text("[A B Sel St Up Dn Lf Rt]: %d %d %d %d %d %d %d %d",
            (buttons>>0)&1, (buttons>>1)&1, (buttons>>2)&1, (buttons>>3)&1,
            (buttons>>4)&1, (buttons>>5)&1, (buttons>>6)&1, (buttons>>7)&1);
        ImGui::End();

        // Registers window
        ImGui::Begin("Registers");
        ImGui::Text("PC: %04X", cpu.PC);
        ImGui::Text("SP: %04X", cpu.SP);
        ImGui::Text("A: %02X", cpu.A);
        ImGui::Text("X: %02X", cpu.X);
        ImGui::Text("Y: %02X", cpu.Y);
        ImGui::Text("Flags: N=%d V=%d B=%d D=%d I=%d Z=%d C=%d",
            cpu.GetFlag(CPU::FLAG_N),
            cpu.GetFlag(CPU::FLAG_V),
            cpu.GetFlag(CPU::FLAG_B),
            cpu.GetFlag(CPU::FLAG_D),
            cpu.GetFlag(CPU::FLAG_I),
            cpu.GetFlag(CPU::FLAG_Z),
            cpu.GetFlag(CPU::FLAG_C));
        ImGui::End();

        // Memory view
        DrawMemoryView(bus, memBase);

        if (show_demo_window){
            ImGui::ShowDemoWindow(&show_demo_window);
        }
        

            // PPU: render a full frame (256x240) and display it
        if (liveRender && ppu.PopFrame(ppuPixels, ppuW, ppuH)) {

            glBindTexture(GL_TEXTURE_2D, ppuTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ppuW, ppuH, 0, GL_RGBA, GL_UNSIGNED_BYTE, ppuPixels.data());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            if(liveRender)
            {
            
            ImGui::Begin("PPU");
            ImGui::Image((void*)(intptr_t)ppuTex, ImVec2((float)ppuW * 2.0f, (float)ppuH * 2.0f));

            // Debug info: registers, palette, OAM, VRAM head
            ImGui::Separator();
            ImGui::Text("PPUCTRL: 0x%02X  PPUMASK: 0x%02X  PPUSTATUS: 0x%02X", ppu.GetPPUCTRL(), ppu.GetPPUMASK(), ppu.GetPPUSTATUS());
            ImGui::Text("Sprite height: %d", ppu.GetSpriteHeight());

            const uint8_t* pal = ppu.GetPaletteRam();
            ImGui::Text("Palette (0x3F00..):");
            ImGui::SameLine();
            for (int i = 0; i < 16; ++i) {
                ImGui::SameLine(); ImGui::Text("%02X", pal[i]);
            }
            ImGui::NewLine();

            const uint8_t* oam = ppu.GetOAM();
            ImGui::Text("OAM (Y,T,A,X) first 16 sprites:");
            for (int i = 0; i < 16; ++i) {
                int b = i * 4;
                ImGui::Text("%02d: %02X %02X %02X %02X", i, oam[b], oam[b+1], oam[b+2], oam[b+3]);
            }

            const uint8_t* v = ppu.GetVRAM();
            ImGui::Text("VRAM[0..63]:");
            for (int i = 0; i < 64; ++i) {
                if ((i % 16) == 0) ImGui::NewLine();
                ImGui::SameLine(); ImGui::Text("%02X", v[i]);
            }

            ImGui::Separator();
            ImGui::Text("Pattern Tables (palette group %d):", patternPaletteGroup);
            ImGui::SameLine();
            ImGui::SliderInt("PaletteGroup", &patternPaletteGroup, 0, 3);

            std::vector<uint32_t> patPixels;
            int pw = 0, ph = 0;
            if (ppu.RenderPatternTable(0, patPixels, pw, ph, patternPaletteGroup)) {
                glBindTexture(GL_TEXTURE_2D, patternTex);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pw, ph, 0, GL_RGBA, GL_UNSIGNED_BYTE, patPixels.data());
                ImGui::Image((void*)(intptr_t)patternTex, ImVec2((float)pw * 1.5f, (float)ph * 1.5f));
            }
            if (ppu.RenderPatternTable(1, patPixels, pw, ph, patternPaletteGroup)) {
                glBindTexture(GL_TEXTURE_2D, patternTex);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pw, ph, 0, GL_RGBA, GL_UNSIGNED_BYTE, patPixels.data());
                ImGui::SameLine();
                ImGui::Image((void*)(intptr_t)patternTex, ImVec2((float)pw * 1.5f, (float)ph * 1.5f));
            }

            ImGui::End();
            }

        }


        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
        // Cleanup
    if (ppuTex) {
        glDeleteTextures(1, &ppuTex);
        ppuTex = 0;
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}





