#include <string>
#include <iostream>

#include "Chip8.h"

int main(int argc, char* argv[])
{
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " <ROM-PATH>" << std::endl;
		std::exit(EXIT_FAILURE);
	}

	std::string romFile = argv[1];

	Chip8 emulator;

	emulator.LoadGame(romFile);

	emulator.Run();
	
	std::cout << "Bye Bye!" << std::endl;

	return 0;
}