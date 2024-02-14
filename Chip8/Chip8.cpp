#include "Chip8.h"

Chip8::Chip8()
{
	m_pc = START_ADDRESS;
	m_memory = {};
	m_V = {};
}

void Chip8::LoadGame(const std::string& gameName)
{
	std::ifstream gameFile(gameName, std::ifstream::binary);

	if (gameFile.is_open()) {
		gameFile.seekg(0, gameFile.end);
		int fileSize = gameFile.tellg();
		gameFile.seekg(0, gameFile.beg);

		char* buffer = new char[fileSize];

		gameFile.read(buffer, fileSize);

		gameFile.close();

		for (int i = 0; i < fileSize; i++) {
			m_memory[m_pc++] = buffer[i];
		}

		m_pc = START_ADDRESS;

		delete[] buffer;
	}
}
