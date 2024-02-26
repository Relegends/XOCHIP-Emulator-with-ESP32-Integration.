#include "Chip8.h"

Chip8::Chip8()
{
	m_pc = START_ADDRESS;
	m_memory = {};
	m_V = {};
	m_stack = {};
}

uint16_t Chip8::Fetch()
{
	uint16_t opcode = 0x0000;

	opcode = m_memory[m_pc] << 8 | m_memory[m_pc + 1];

	m_pc += 2;

	return opcode;
}

void Chip8::Decode(uint16_t opcode)
{	
	m_decoder.type = (opcode >> 12) & 0x000F;
	m_decoder.nnn = opcode & 0x0FFF;
	m_decoder.kk = opcode & 0x00FF;
	m_decoder.n = opcode & 0x000F;
	m_decoder.x = (opcode >> 8) & 0x000F;
	m_decoder.y = (opcode >> 4) & 0x000F;

	switch (m_decoder.type)
	{
	case 0x0:
		if (m_decoder.nnn == 0x0E0) Fun_00E0();
		else if (m_decoder.nnn == 0x0EE) Fun_00EE();
		break;
	case 0x1:
		Fun_1nnn();
		break;
	case 0x2:
		Fun_2nnn();
		break;
	case 0x3:
		Fun_3xkk();
		break;
	case 0x4:
		Fun_4xkk();
		break;
	case 0x5:
		Fun_5xy0();
		break;
	case 0x6:
		Fun_6xkk();
		break;
	case 0x7:
		Fun_7xkk();
		break;
	case 0x8:
		switch (m_decoder.n)
		{
		case 0x0:
			Fun_8xy0();
			break;
		case 0x1:
			Fun_8xy1();
			break;
		case 0x2:
			Fun_8xy2();
			break;
		case 0x3:
			Fun_8xy3();
			break;
		case 0x4:
			Fun_8xy4();
			break;
		case 0x5:
			Fun_8xy5();
			break;
		case 0x6:
			Fun_8xy6();
			break;
		case 0x7:
			Fun_8xy7();
			break;
		case 0xE:
			Fun_8xyE();
			break;
		default:
			// ERROR
			break;
		}
	case 0x9:
		Fun_9xy0();
		break;
	case 0xA:
		Fun_Annn();
		break;
	case 0xB:
		Fun_Bnnn();
		break;
	case 0xC:
		Fun_Cxkk();
		break;
	default:
		// ERROR
		break;
	}
}

void Chip8::LoadGame(const std::string& gameName)
{
	std::ifstream gameFile(gameName, std::ifstream::binary);

	if (gameFile.is_open()) {
		gameFile.seekg(0, gameFile.end);
		std::streamoff fileSize = gameFile.tellg();
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
