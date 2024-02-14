#pragma once
#include <stdint.h>
#include <fstream>
#include <string>
#include <array>

class Chip8
{
public:

	Chip8();

	void LoadGame(const std::string& gameName);

private:
	std::array<uint8_t, 16> m_V;
	std::array<uint8_t, 0x1000> m_memory;


	uint16_t m_indexRegister; // Only the lowest 12 bits are used. Points to locations in the memory (0x000 - 0xFFF)
	uint16_t m_pc; // Program Counter. Stores the currently executing address.

	std::unique_ptr<uint16_t> m_sp;
	std::array<uint16_t, 16> m_stack;

	uint8_t m_delayTimer;
	uint8_t m_soundTimer;

	const uint16_t START_ADDRESS = 0x200;
};

