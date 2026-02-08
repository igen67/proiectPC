#include "headers/cpu.h"
#include "headers/table.h"



// Use Bus instead of Mem
#include "headers/bus.h"

/*
Old main (commented out for safety):
int main()
{
	Bus bus{};
	CPU cpu{};
	u32 Cycles = 0;

	std::string filePath;
	std::cerr << "Pick program to execute"<<std::endl;
	std::cin >> filePath;

	// If file has an iNES header, load as PRG ROM; otherwise load it into RAM (legacy functional tests)
	{
		#include <fstream>
		std::ifstream f(filePath, std::ios::binary);
		char header[4] = {0};
		bool loadedPRG = false;
		if (f && f.read(header, 4) && std::string(header, header+4) == "NES\x1A") {
			loadedPRG = bus.LoadPRGFromFile(filePath);
		}
		if (!loadedPRG) {
			std::cerr << "Loading into RAM image" << std::endl;
			bus.ram.LoadMachineCodeFromFile(filePath);
		}
	}

	cpu.Reset(bus);
	InitializeInstructionTable();
	cpu.Execute(Cycles, bus);
	system("pause");
}
*/

// New main: supports 'gui' mode (./proiectPC gui [rom]) when GUI is available; otherwise REPL mode
int main(int argc, char** argv)
{
	Bus bus{};
	CPU cpu{};
	u32 Cycles = 0;

	// Detect GUI mode first: 'gui' as first arg
	bool wantGui = false;
	std::string filePath;
	if (argc > 1 && std::string(argv[1]) == "gui") {
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

	// If GUI requested and available, start it (and optionally pre-load ROM)
	#if __has_include("gui.h")
	#include "gui.h"
	if (wantGui) {
		if (!filePath.empty()) loadFile(filePath);
		InitializeInstructionTable();
		cpu.Reset(bus);

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
			std::cerr << "PC: 0x" << std::hex << cpu.PC << std::dec << " SP: 0x" << std::hex << cpu.SP << std::dec << std::endl;
		}
	}

	std::cerr << "Exiting." << std::endl;
	return 0;
}


