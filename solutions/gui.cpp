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
    ppu.Reset();
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
    bool running = false; // auto-start to measure emulation speed
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
        if (ImGui::Button("Step")) {
            cpu.Step(cycles, bus);
        }
        ImGui::SameLine();
        if (ImGui::Button(running ? "Pause" : "Run")) {
            running = !running;
        }
        ImGui::SameLine();
        if (ImGui::Button("Render Frame Now")) {
            if (ppu.RenderFrame(ppuPixels, ppuW, ppuH)) {
                glBindTexture(GL_TEXTURE_2D, ppuTex);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ppuW, ppuH, 0, GL_RGBA, GL_UNSIGNED_BYTE, ppuPixels.data());
            }
        }
        ImGui::SameLine();
        ImGui::Checkbox("Live Render", &liveRender);
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
        ImGui::End();

        // Registers window
        ImGui::Begin("Registers");
        ImGui::Text("PC: %04X", cpu.PC);
        ImGui::Text("SP: %04X", cpu.SP);
        ImGui::Text("A: %02X", cpu.A);
        ImGui::Text("X: %02X", cpu.X);
        ImGui::Text("Y: %02X", cpu.Y);
        ImGui::Text("Flags: N=%d V=%d B=%d D=%d I=%d Z=%d C=%d", cpu.N, cpu.V, cpu.B, cpu.D, cpu.I, cpu.Z, cpu.C);
        ImGui::End();

        // Memory view
        DrawMemoryView(bus, memBase);

        if (running) {
            // Throttle emulation to NES frame rate (60Hz)
            static auto lastFrameTime = std::chrono::steady_clock::now();
            auto nowFrame = std::chrono::steady_clock::now();
            double elapsedSec = std::chrono::duration<double>(nowFrame - lastFrameTime).count();
            if (elapsedSec < (1.0 / 60.0)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            lastFrameTime = nowFrame;

            // Run enough cycles for one NES frame (about 29780 cycles)
            uint32_t targetCycles = 29780;
            uint32_t executed = 0;
            while (executed < targetCycles && !glfwWindowShouldClose(window)) {
                u32 before = cycles;
                cpu.Step(cycles, bus);
                executed += (cycles - before);
            }

            // Only render when a new frame is ready
            if (liveRender && ppu.PopFrame(ppuPixels, ppuW, ppuH)) {
                glBindTexture(GL_TEXTURE_2D, ppuTex);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ppuW, ppuH, 0, GL_RGBA, GL_UNSIGNED_BYTE, ppuPixels.data());
            }
        }

        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // PPU: render a full frame (256x240) and display it
        if (ppu.RenderFrame(ppuPixels, ppuW, ppuH)) {
            glBindTexture(GL_TEXTURE_2D, ppuTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ppuW, ppuH, 0, GL_RGBA, GL_UNSIGNED_BYTE, ppuPixels.data());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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
