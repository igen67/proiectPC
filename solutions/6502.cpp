#include "headers/cpu.h"
#include "headers/table.h"
#include "headers/input.h"

#include <cctype>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>



// Use Bus instead of Mem
#include "headers/bus.h"


struct ExpectedTrace {
	uint16_t pc = 0;
	uint8_t op1 = 0;
	uint8_t op2 = 0;
	uint8_t op3 = 0;
	uint8_t a = 0;
	uint8_t x = 0;
	uint8_t y = 0;
	uint8_t p = 0;
	uint8_t sp = 0;
	uint64_t cycles = 0;
};

static int HexVal(char c) {
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
	if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
	return -1;
}

static bool ParseByteAt(const std::string& s, size_t pos, uint8_t& out) {
	if (pos + 1 >= s.size()) return false;
	int hi = HexVal(s[pos]);
	int lo = HexVal(s[pos + 1]);
	if (hi < 0 || lo < 0) return false;
	out = static_cast<uint8_t>((hi << 4) | lo);
	return true;
}

static bool ParseWordAt(const std::string& s, size_t pos, uint16_t& out) {
	if (pos + 3 >= s.size()) return false;
	int h1 = HexVal(s[pos]);
	int h2 = HexVal(s[pos + 1]);
	int h3 = HexVal(s[pos + 2]);
	int h4 = HexVal(s[pos + 3]);
	if (h1 < 0 || h2 < 0 || h3 < 0 || h4 < 0) return false;
	out = static_cast<uint16_t>((h1 << 12) | (h2 << 8) | (h3 << 4) | h4);
	return true;
}

static bool ParseExpectedTraceLine(const std::string& line, ExpectedTrace& out) {
	if (line.size() < 4 || !std::isxdigit(static_cast<unsigned char>(line[0]))) return false;
	if (!ParseWordAt(line, 0, out.pc)) return false;

	// Byte columns in nestest log are fixed-width.
	ParseByteAt(line, 6, out.op1);
	ParseByteAt(line, 9, out.op2);
	ParseByteAt(line, 12, out.op3);

	auto findByte = [&](const char* tag, uint8_t& value) {
		size_t pos = line.find(tag);
		if (pos == std::string::npos) return false;
		pos += std::strlen(tag);
		return ParseByteAt(line, pos, value);
	};

	findByte("A:", out.a);
	findByte("X:", out.x);
	findByte("Y:", out.y);
	findByte("P:", out.p);
	findByte("SP:", out.sp);

	size_t cycPos = line.find("CYC:");
	if (cycPos != std::string::npos) {
		cycPos += 4;
		while (cycPos < line.size() && line[cycPos] == ' ') ++cycPos;
		out.cycles = std::strtoull(line.c_str() + cycPos, nullptr, 10);
	}

	return true;
}

// New main: supports 'gui' mode (./proiectPC gui [rom]) when GUI is available; otherwise REPL mode
int main(int argc, char** argv)
{
	Bus bus{};
	CPU cpu{};
	u32 Cycles = 0;
	std::string filePath;

	bool traceCompare = false;
	std::string traceLogPath = "nesTests/goodlog.txt";
	int traceMaxLines = 5000;

	// Detect trace compare mode first: 'trace' as first arg
	if (argc > 1 && std::string(argv[1]) == "trace") {
		traceCompare = true;
		if (argc > 2) filePath = argv[2];
		else filePath = "nesTests/nestest.nes";
		if (argc > 3) traceLogPath = argv[3];
		if (argc > 4) traceMaxLines = std::max(1, std::stoi(argv[4]));
	}

	// Detect GUI mode first: 'gui' as first arg
	bool wantGui = false;
	if (!traceCompare && argc > 1 && std::string(argv[1]) == "gui") {
		wantGui = true;
		if (argc > 2) filePath = argv[2];
	} else if (argc > 1) {
		filePath = argv[1];
	} else {
		std::cerr << "Pick program to execute: ";
		std::getline(std::cin, filePath);
		if (filePath.empty()) std::cin >> filePath;
	}

	// Function to load a file into PRG or RAM
	auto loadFile = [&](const std::string& path) {
		if (path.empty()) return;
		#include <fstream>
		std::ifstream f(path, std::ios::binary);
		char header[4] = {0};
		bool loadedPRG = false;
		if (f && f.read(header, 4) && std::string(header, header+4) == "NES\x1A") {
			loadedPRG = bus.LoadPRGFromFile(path);
		}
		if (!loadedPRG) {
			std::cerr << "Loading into RAM image" << std::endl;
			bus.ram.LoadMachineCodeFromFile(path);
		}
	};

	// Trace-compare mode (nestest)
	if (traceCompare) {
		if (!filePath.empty()) loadFile(filePath);
		InitializeInstructionTable();
		bus.AttachCPU(&cpu);

		// Force nestest initial state (matches goodlog.txt)
		cpu.PC = 0xC000;
		cpu.SP = 0xFD;
		cpu.A = 0x00;
		cpu.X = 0x00;
		cpu.Y = 0x00;
		cpu.P = 0x24;
		Cycles = 7;

		std::ifstream goodlog(traceLogPath);
		if (!goodlog.is_open()) {
			std::cerr << "Failed to open trace log: " << traceLogPath << "\n";
			return 1;
		}

		std::string line;
		int lineNo = 0;
		int matched = 0;
		while (std::getline(goodlog, line)) {
			ExpectedTrace expected;
			if (!ParseExpectedTraceLine(line, expected)) continue;
			lineNo++;

			uint8_t op1 = bus.read(cpu.PC);
			uint8_t op2 = bus.read(cpu.PC + 1);
			uint8_t op3 = bus.read(cpu.PC + 2);

			bool mismatch = false;
			if (cpu.PC != expected.pc) mismatch = true;
			if (cpu.A != expected.a || cpu.X != expected.x || cpu.Y != expected.y) mismatch = true;
			if (cpu.P != expected.p || cpu.SP != expected.sp) mismatch = true;
			if (op1 != expected.op1 || op2 != expected.op2 || op3 != expected.op3) mismatch = true;
			if (expected.cycles != 0 && Cycles != expected.cycles) mismatch = true;

			if (mismatch) {
				std::cerr << "Trace mismatch at line " << lineNo << "\n";
				std::cerr << "Expected PC=0x" << std::hex << expected.pc
					<< " A=" << int(expected.a) << " X=" << int(expected.x)
					<< " Y=" << int(expected.y) << " P=" << int(expected.p)
					<< " SP=" << int(expected.sp) << " CYC=" << std::dec << expected.cycles << "\n";
				std::cerr << "Actual   PC=0x" << std::hex << cpu.PC
					<< " A=" << int(cpu.A) << " X=" << int(cpu.X)
					<< " Y=" << int(cpu.Y) << " P=" << int(cpu.P)
					<< " SP=" << int(cpu.SP) << " CYC=" << std::dec << Cycles << "\n";
				std::cerr << "Bytes    exp=" << std::hex << int(expected.op1) << " " << int(expected.op2)
					<< " " << int(expected.op3) << " act=" << int(op1) << " " << int(op2) << " " << int(op3) << std::dec << "\n";
				return 1;
			}

			cpu.Execute(Cycles, bus);
			matched++;
			if (traceMaxLines > 0 && matched >= traceMaxLines) break;
		}

		std::cout << "Trace compare passed for " << matched << " lines." << std::endl;
		return 0;
	}

	// If GUI requested and available, start it (and optionally pre-load ROM)
	#if __has_include("gui.h")
	#include "gui.h"
	if (wantGui) {
		if (!filePath.empty()) loadFile(filePath);
		InitializeInstructionTable();
		// Attach CPU to bus for mapper IRQs and other interactions
		bus.AttachCPU(&cpu);
		cpu.Reset(bus);
		// SANITY CHECK — REMOVE AFTER TEST

		RunGUI(cpu, bus);
		return 0;
	}
	#else
	if (wantGui) {
		std::cerr << "GUI support not compiled in. Rebuild with BUILD_GUI=ON.\n";
	}
	#endif

	// Non-GUI / REPL mode
	if (!filePath.empty()) loadFile(filePath);

	InitializeInstructionTable();
	// Attach CPU to bus so mappers can signal IRQs even in non-GUI runs
	bus.AttachCPU(&cpu);
	cpu.Reset(bus);

	std::cerr << "Commands: r=run, s=step, p=print regs, q=quit\n";
	while (true) {
		std::cerr << "> ";
		char cmd = 0;
		if (!(std::cin >> cmd)) break;
		if (cmd == 'q') break;
		else if (cmd == 'r') cpu.Execute(Cycles, bus);
		else if (cmd == 'p') {
			cpu.printReg('A'); cpu.printReg('X'); cpu.printReg('Y');
			std::cerr << "PC: 0x" << std::hex << cpu.PC << std::dec << " SP: 0x" << std::hex << int(cpu.SP) << std::dec << std::endl;
		}
	}

	std::cerr << "Exiting." << std::endl;
	return 0;
}


