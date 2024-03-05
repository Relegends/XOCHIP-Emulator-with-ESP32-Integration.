#include <string.h>
#include <iostream>
#include <memory>
#include <SDL.h>

#include "Chip8.h"
#include "Display.h"

int main(int argc, char* argv[])
{
	/*if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " <ROM>" << std::endl;
		std::exit(EXIT_FAILURE);
	}

	const std::string romFile = argv[1];*/

	const std::string romFile = "./testRom.ch8";

	Chip8 emulator;

	emulator.LoadGame(romFile);
	
	return 0;
}