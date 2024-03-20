#include "Chip8.h"
#include <iostream>

Chip8::Chip8()
{
	m_pc = START_ADDRESS;
	m_memory = {};
	m_V = {};

	m_stack = {};
	m_sp = m_stack.begin();

	m_decoder = {};

	if (!m_display.Init("Chip 8", SCREEN_WIDTH, SCREEN_HEIGHT))
		exit(-1);

	std::srand(std::time(0));
}

uint16_t Chip8::Fetch()
{
	uint16_t opcode = 0x0000;

	opcode = m_memory[m_pc] << 8 | m_memory[m_pc + 1];

	m_pc += 2;

	return opcode;
}

const std::function<void()> Chip8::Decode(uint16_t opcode)
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
		if (m_decoder.nnn == 0x0E0)  return std::bind(&Chip8::Fun_00E0, this);
		else if (m_decoder.nnn == 0x0EE)  return std::bind(&Chip8::Fun_00EE, this);
	case 0x1:
		return std::bind(&Chip8::Fun_1nnn, this);
	case 0x2:
		return std::bind(&Chip8::Fun_2nnn, this);
	case 0x3:
		return std::bind(&Chip8::Fun_3xkk, this);
	case 0x4:
		return std::bind(&Chip8::Fun_4xkk, this);
	case 0x5:
		return std::bind(&Chip8::Fun_5xy0, this);
	case 0x6:
		return std::bind(&Chip8::Fun_6xkk, this);
	case 0x7:
		return std::bind(&Chip8::Fun_7xkk, this);
	case 0x8:
		switch (m_decoder.n)
		{
		case 0x0:
			return std::bind(&Chip8::Fun_8xy0, this);
		case 0x1:
			return std::bind(&Chip8::Fun_8xy1, this);
		case 0x2:
			return std::bind(&Chip8::Fun_8xy2, this);
		case 0x3:
			return std::bind(&Chip8::Fun_8xy3, this);
		case 0x4:
			return std::bind(&Chip8::Fun_8xy4, this);
		case 0x5:
			return std::bind(&Chip8::Fun_8xy5, this);
		case 0x6:
			return std::bind(&Chip8::Fun_8xy6, this);
		case 0x7:
			return std::bind(&Chip8::Fun_8xy7, this);
		case 0xE:
			return std::bind(&Chip8::Fun_8xyE, this);
		}
	case 0x9:
		return std::bind(&Chip8::Fun_9xy0, this);
	case 0xA:
		return std::bind(&Chip8::Fun_Annn, this);
	case 0xB:
		return std::bind(&Chip8::Fun_Bnnn, this);
	case 0xC:
		return std::bind(&Chip8::Fun_Cxkk, this);
	}

	// If the opcode does not exit -> ERROR
	std::cout << "ERROR: Decoded opcode not implemented!" << std::endl;
	exit(-1);
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

void Chip8::Simulate_Cycle()
{
	uint16_t opcode = Chip8::Fetch();
	Chip8::Decode(opcode);


}

void Chip8::Fun_00E0()
{
	printf("CLS");
	m_display.CLS();
}

void Chip8::Fun_00EE()
{
	printf("CALL %4x", m_decoder.nnn);
	m_pc = m_stack.front();
	m_sp--;
}

void Chip8::Fun_1nnn()
{
	m_pc = m_decoder.nnn;
}

void Chip8::Fun_2nnn()
{
	m_sp++;
}

void Chip8::Fun_3xkk()
{
	if (m_V[m_decoder.x] == m_decoder.kk)
		m_pc += 2;
}

void Chip8::Fun_4xkk()
{
	if (m_V[m_decoder.x] != m_decoder.kk)
		m_pc += 2;
}

void Chip8::Fun_5xy0()
{
	if (m_V[m_decoder.x] == m_V[m_decoder.y])
		m_pc += 2;
}

void Chip8::Fun_6xkk()
{
	if (m_V[m_decoder.x] == m_decoder.kk)
		m_pc += 2;
}

void Chip8::Fun_7xkk()
{
	m_V[m_decoder.x] += m_decoder.kk;
}

void Chip8::Fun_8xy0()
{
	m_V[m_decoder.x] = m_V[m_decoder.y];
}

void Chip8::Fun_8xy1()
{
	m_V[m_decoder.x] |= m_V[m_decoder.y];
}

void Chip8::Fun_8xy2()
{
	m_V[m_decoder.x] &= m_V[m_decoder.y];
}

void Chip8::Fun_8xy3()
{
	m_V[m_decoder.x] ^= m_V[m_decoder.y];
}

void Chip8::Fun_8xy4()
{
	uint16_t result = m_V[m_decoder.x] + m_V[m_decoder.y];
	m_V[m_decoder.x] = result & 0x00FF;

	m_V[0xF] = (result > 0xFF);
}

void Chip8::Fun_8xy5()
{
	uint16_t result = m_V[m_decoder.y] - m_V[m_decoder.x];

	m_V[0xF] = (m_V[m_decoder.x] > m_V[m_decoder.y]);
	m_V[m_decoder.x] = result & 0x00FF;
}

void Chip8::Fun_8xy6()
{
	m_V[0xF] = ((m_V[m_decoder.x] & 0x01) == 1);
	m_V[m_decoder.x] /= 2;
}

void Chip8::Fun_8xy7()
{
	uint16_t result = m_V[m_decoder.x] - m_V[m_decoder.y];

	m_V[0xF] = (m_V[m_decoder.y] > m_V[m_decoder.x]);
	m_V[m_decoder.x] = result & 0x00FF;
}

void Chip8::Fun_8xyE()
{
	m_V[0xF] = ((m_V[m_decoder.x] >> 7) == 1);
	m_V[m_decoder.x] *= 2;
}

void Chip8::Fun_9xy0()
{
	if (m_V[m_decoder.x] != m_V[m_decoder.y])
		m_pc += 2;
}

void Chip8::Fun_Annn()
{
	m_indexRegister = m_decoder.nnn;
}

void Chip8::Fun_Bnnn()
{
	m_pc = m_decoder.nnn + m_V[m_decoder.x];
}

void Chip8::Fun_Cxkk()
{
	uint8_t random = std::rand() % 256;

	m_V[m_decoder.x] &= random;
}

void Chip8::Fun_Dxyn()
{
	uint8_t xPos = m_V[m_decoder.x];
	uint8_t yPos = m_V[m_decoder.y];

	m_V[0xF] = 0;

	for (int row = 0; row < m_decoder.n; row++)
	{
		uint8_t spriteRow = m_memory[m_indexRegister + row];
		for (int col = 0; col < 8; col++)
		{
			bool pixel = (spriteRow >> col) & 0x1;
			bool conflict = m_display.ChangePixelValue(xPos + col, yPos + row, pixel);
			if (conflict)
				m_V[0xF] = 1;
		}
	}
}

void Chip8::Fun_Ex9E()
{
	//aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
	/*if ()
		m_pc += 2;*/
}

void Chip8::Fun_Fx07()
{
	m_V[m_decoder.x] = m_delayTimer;
}
