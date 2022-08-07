#pragma once
#include <stdint.h>
#include <fstream>
#include <string>

class Chip8
{
	uint8_t V[16];
	uint8_t memory[4096];

	uint8_t* sp;

	uint16_t indexRegister;
	uint16_t pc;

	uint16_t stack[16];
	
	uint8_t delay_timer;
	uint8_t sound_timer;

	uint32_t display[64 * 32];

	const uint16_t START_ADDRESS = 0x200;

public:

	Chip8();

	void LoadGame(const std::string& gameName);
};

