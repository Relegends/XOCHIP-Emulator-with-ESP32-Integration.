#include "Chip8.h"
#include <iostream>
#include <chrono>
#include <thread>

Chip8::Chip8() :
	m_V({ 0 }),
	m_pc(0x200), // Start Address
	m_memory({ 0 }),
	m_stack({ 0 }),
	m_sp(m_stack.begin()),
	m_config("./Config/Config.toml"),
	m_display("Chip 8", m_config.GetWindowWidth(), m_config.GetWindowHeight(), m_config.GetDisplayWidth(), m_config.GetDisplayHeight(),
		m_config.GetDisplayMainColor(), m_config.GetDisplayBackgroundColor()),
	m_keyboard({ m_config.GetInputMap() }),
	m_decoder({ 0 }),
	m_interpreter((Chip8::INTERPRETER)m_config.GetInterpretedMode()),
	m_instructionCount(0),
	m_ips(m_config.GetIPS())
{
	if (!m_display.Init()) exit(-1);
	std::srand(static_cast<unsigned int>(std::time(0)));

	InitQuirks();

	LoadFontSet();
}

void Chip8::LoadFontSet()
{
	switch (m_interpreter)
	{
	case Chip8::VIP:
		m_memory =
		{
			0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
			0x20, 0x60, 0x20, 0x20, 0x70, // 1
			0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
			0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
			0x90, 0x90, 0xF0, 0x10, 0x10, // 4
			0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
			0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
			0xF0, 0x10, 0x20, 0x40, 0x40, // 7
			0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
			0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
			0xF0, 0x90, 0xF0, 0x90, 0x90, // A
			0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
			0xF0, 0x80, 0x80, 0x80, 0xF0, // C
			0xE0, 0x90, 0x90, 0x90, 0xE0, // D
			0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
			0xF0, 0x80, 0xF0, 0x80, 0x80 // F
		};
		break;
	case Chip8::SCHIP:
		m_memory =
		{
			0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
			0x20, 0x60, 0x20, 0x20, 0x70, // 1
			0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
			0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
			0x90, 0x90, 0xF0, 0x10, 0x10, // 4
			0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
			0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
			0xF0, 0x10, 0x20, 0x40, 0x40, // 7
			0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
			0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
			0xF0, 0x90, 0xF0, 0x90, 0x90, // A
			0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
			0xF0, 0x80, 0x80, 0x80, 0xF0, // C
			0xE0, 0x90, 0x90, 0x90, 0xE0, // D
			0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
			0xF0, 0x80, 0xF0, 0x80, 0x80, // F

			//High res font
			0x3C, 0x7E, 0xE7, 0xC3, 0xC3, 0xC3, 0xC3, 0xE7, 0x7E, 0x3C, // 0
			0x18, 0x38, 0x58, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, // 1
			0x3E, 0x7F, 0xC3, 0x06, 0x0C, 0x18, 0x30, 0x60, 0xFF, 0xFF, // 2
			0x3C, 0x7E, 0xC3, 0x03, 0x0E, 0x0E, 0x03, 0xC3, 0x7E, 0x3C, // 3
			0x06, 0x0E, 0x1E, 0x36, 0x66, 0xC6, 0xFF, 0xFF, 0x06, 0x06, // 4
			0xFF, 0xFF, 0xC0, 0xC0, 0xFC, 0xFE, 0x03, 0xC3, 0x7E, 0x3C, // 5
			0x3E, 0x7C, 0xC0, 0xC0, 0xFC, 0xFE, 0xC3, 0xC3, 0x7E, 0x3C, // 6
			0xFF, 0xFF, 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x60, 0x60, // 7
			0x3C, 0x7E, 0xC3, 0xC3, 0x7E, 0x7E, 0xC3, 0xC3, 0x7E, 0x3C, // 8
			0x3C, 0x7E, 0xC3, 0xC3, 0x7F, 0x3F, 0x03, 0x03, 0x3E, 0x7C  // 9
		};
		break;
	case Chip8::OCTO:
		m_memory =
		{
			0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
			0x20, 0x60, 0x20, 0x20, 0x70, // 1
			0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
			0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
			0x90, 0x90, 0xF0, 0x10, 0x10, // 4
			0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
			0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
			0xF0, 0x10, 0x20, 0x40, 0x40, // 7
			0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
			0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
			0xF0, 0x90, 0xF0, 0x90, 0x90, // A
			0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
			0xF0, 0x80, 0x80, 0x80, 0xF0, // C
			0xE0, 0x90, 0x90, 0x90, 0xE0, // D
			0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
			0xF0, 0x80, 0xF0, 0x80, 0x80, // F

			//High res font
			0x3C, 0x7E, 0xE7, 0xC3, 0xC3, 0xC3, 0xC3, 0xE7, 0x7E, 0x3C, // 0
			0x18, 0x38, 0x58, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, // 1
			0x3E, 0x7F, 0xC3, 0x06, 0x0C, 0x18, 0x30, 0x60, 0xFF, 0xFF, // 2
			0x3C, 0x7E, 0xC3, 0x03, 0x0E, 0x0E, 0x03, 0xC3, 0x7E, 0x3C, // 3
			0x06, 0x0E, 0x1E, 0x36, 0x66, 0xC6, 0xFF, 0xFF, 0x06, 0x06, // 4
			0xFF, 0xFF, 0xC0, 0xC0, 0xFC, 0xFE, 0x03, 0xC3, 0x7E, 0x3C, // 5
			0x3E, 0x7C, 0xC0, 0xC0, 0xFC, 0xFE, 0xC3, 0xC3, 0x7E, 0x3C, // 6
			0xFF, 0xFF, 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x60, 0x60, // 7
			0x3C, 0x7E, 0xC3, 0xC3, 0x7E, 0x7E, 0xC3, 0xC3, 0x7E, 0x3C, // 8
			0x3C, 0x7E, 0xC3, 0xC3, 0x7F, 0x3F, 0x03, 0x03, 0x3E, 0x7C  // 9
		};
		break;
	case Chip8::XOCHIP:
		break;
	default:
		std::cout << "NOT VALID INTERPRETER!!!" << std::endl;
		exit(-1);
		break;
	}
}

void Chip8::InitQuirks()
{
	switch (m_interpreter)
	{
	case Chip8::VIP:
		m_VFreset_quirk = true;
		m_Memory_quirk = true;
		m_DisplayWait_quirk = true;
		m_Clipping_quirk = true;
		m_Shifting_quirk = false;
		m_Jumping_quirk = false;
		break;
	case Chip8::SCHIP:
		m_VFreset_quirk = false;
		m_Memory_quirk = false;
		m_DisplayWait_quirk = false;
		m_Clipping_quirk = true;
		m_Shifting_quirk = true;
		m_Jumping_quirk = true;
		break;
	case Chip8::OCTO:
		m_VFreset_quirk = false;
		m_Memory_quirk = true;
		m_DisplayWait_quirk = false;
		m_Clipping_quirk = false;
		m_Shifting_quirk = false;
		m_Jumping_quirk = false;
		break;
	case Chip8::XOCHIP:
		m_VFreset_quirk = false;
		m_Memory_quirk = true;
		m_DisplayWait_quirk = false;
		m_Clipping_quirk = false;
		m_Shifting_quirk = false;
		m_Jumping_quirk = false;
		break;
	default:
		break;
	}

	UpdateQuirk(m_VFreset_quirk, m_config.GetVFresetQuirk());
	UpdateQuirk(m_Memory_quirk, m_config.GetMemoryQuirk());
	UpdateQuirk(m_DisplayWait_quirk, m_config.GetDisplayWaitQuirk());
	UpdateQuirk(m_Clipping_quirk, m_config.GetClippingQuirk());
	UpdateQuirk(m_Shifting_quirk, m_config.GetShiftingQuirk());
	UpdateQuirk(m_Jumping_quirk, m_config.GetJumpingQuirk());
}

void Chip8::UpdateQuirk(bool& quirkMember, const int configValue)
{
	if (configValue != -1)
		quirkMember = configValue;
}

uint16_t Chip8::Fetch()
{
	uint16_t opcode = 0x0000;

	opcode = m_memory[m_pc] << 8 | m_memory[m_pc + 1];

	m_pc += 2;

	return opcode;
}

std::function<void()> Chip8::Decode(uint16_t opcode)
{
	m_decoder.type = (opcode >> 12) & 0x000F;
	m_decoder.nnn = opcode & 0x0FFF;
	m_decoder.kk = opcode & 0x00FF;
	m_decoder.n = opcode & 0x000F;
	m_decoder.x = (opcode >> 8) & 0x000F;
	m_decoder.y = (opcode >> 4) & 0x000F;

	if (m_decoder.kk != 0x0a)
	{
		printf("I = 0x%.4X ", m_indexRegister);
		for (uint8_t i = 0; i < m_V.size(); i++)
			printf("V%.1X = 0x%.2X ", i, m_V[i]);
		printf("PC: % .4X OPCODE: % .4X -> ", m_pc - 2, opcode);
	}


	switch (m_decoder.type)
	{
	case 0x0:
		switch (m_decoder.y)
		{
		case 0xC:
			return std::bind(&Chip8::Fun_00Cn, this);
		case 0xD:
			return std::bind(&Chip8::Fun_00Dn, this);
		}
		switch (m_decoder.kk)
		{
		case 0xE0:
			return std::bind(&Chip8::Fun_00E0, this);
		case 0xEE:
			return std::bind(&Chip8::Fun_00EE, this);
		case 0xFB:
			return std::bind(&Chip8::Fun_00FB, this);
		case 0xFC:
			return std::bind(&Chip8::Fun_00FC, this);
		case 0xFE:
			return std::bind(&Chip8::Fun_00FE, this);
		case 0xFF:
			return std::bind(&Chip8::Fun_00FF, this);
		}
		break;
	case 0x1:
		return std::bind(&Chip8::Fun_1nnn, this);
	case 0x2:
		return std::bind(&Chip8::Fun_2nnn, this);
	case 0x3:
		return std::bind(&Chip8::Fun_3xkk, this);
	case 0x4:
		return std::bind(&Chip8::Fun_4xkk, this);
	case 0x5:
		switch (m_decoder.n)
		{
		case 0x0:
			return std::bind(&Chip8::Fun_5xy0, this);
		case 0x2:
			return std::bind(&Chip8::Fun_5xy2, this);
		case 0x3:
			return std::bind(&Chip8::Fun_5xy3, this);
		}
		break;
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
		break;
	case 0x9:
		return std::bind(&Chip8::Fun_9xy0, this);
	case 0xA:
		return std::bind(&Chip8::Fun_Annn, this);
	case 0xB:
		return std::bind(&Chip8::Fun_Bnnn, this);
	case 0xC:
		return std::bind(&Chip8::Fun_Cxkk, this);
	case 0xD:
		return std::bind(&Chip8::Fun_Dxyn, this);
	case 0xE:
		switch (m_decoder.kk)
		{
		case 0x9E:
			return std::bind(&Chip8::Fun_Ex9E, this);
		case 0xA1:
			return std::bind(&Chip8::Fun_ExA1, this);
		}
		break;
	case 0xF:
		switch (m_decoder.kk)
		{
		case 0x00:
			return std::bind(&Chip8::Fun_F000, this);
		case 0x07:
			return std::bind(&Chip8::Fun_Fx07, this);
		case 0x0A:
			return std::bind(&Chip8::Fun_Fx0A, this);
		case 0x15:
			return std::bind(&Chip8::Fun_Fx15, this);
		case 0x18:
			return std::bind(&Chip8::Fun_Fx18, this);
		case 0x1E:
			return std::bind(&Chip8::Fun_Fx1E, this);
		case 0x29:
			return std::bind(&Chip8::Fun_Fx29, this);
		case 0x30:
			return std::bind(&Chip8::Fun_Fx30, this);
		case 0x33:
			return std::bind(&Chip8::Fun_Fx33, this);
		case 0x55:
			return std::bind(&Chip8::Fun_Fx55, this);
		case 0x65:
			return std::bind(&Chip8::Fun_Fx65, this);
		}
		break;
	}

	// If the opcode does not exit -> ERROR
	printf("ERROR: Decoded opcode not implemented! OPCODE: %.4X\n", opcode);
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

		m_pc = 0x200; // Return the pc to the start of the program. 

		delete[] buffer;
	}
}

void Chip8::Simulate_Cycle()
{
	uint16_t opcode = Chip8::Fetch();
	std::function<void()> ExecuteNextInstruction = Chip8::Decode(opcode);

	ExecuteNextInstruction();
}

void Chip8::Run()
{
	std::chrono::high_resolution_clock::time_point start{}, end{};
	std::chrono::microseconds elapsedTime{ 0 };
	std::chrono::microseconds thresholdCPU{ 16667 }; // 16.67 ms

	while (!m_display.IsWindowClose())
	{

		start = std::chrono::high_resolution_clock::now();

		while (m_instructionCount < m_ips)
		{
			m_keyboard.ProcessInput();
			Simulate_Cycle();
			m_keyboard.SavePreviousInputs();
			m_instructionCount++;
		}
		m_instructionCount = 0;

		end = std::chrono::high_resolution_clock::now();

		elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

		std::this_thread::sleep_for(thresholdCPU > elapsedTime ? (thresholdCPU - elapsedTime) : std::chrono::milliseconds(0));

		if (m_delayTimer > 0)
			m_delayTimer--;



		m_display.Update();

	}
}

/*
00CN - SCD N
Scroll display N lines down.
*/
void Chip8::Fun_00Cn()
{
	printf("SCD %d\n", m_decoder.n);
	m_display.ScrollDown(m_decoder.n);
}

/*
00CN - SCU N
Scroll display N lines up.
*/
void Chip8::Fun_00Dn()
{
	printf("SCU %d\n", m_decoder.n);
	m_display.ScrollUp(m_decoder.n);
}

/*
00E0 - CLS
Clear the display.
*/
void Chip8::Fun_00E0()
{
	printf("CLS\n");
	m_display.CLS();
}

/*
00EE - RET
Return from a subroutine.
Return from a subroutine.

The interpreter sets the program counter to the address at the top of the stack, then subtracts 1 from the stack pointer.
*/
void Chip8::Fun_00EE()
{
	printf("RET\n");
	m_sp--;
	m_pc = *m_sp;
}

/*
00FB - SCROLL R N
Scroll display 4 pixels to the right.
*/
void Chip8::Fun_00FB()
{
	printf("SCR\n");
	m_display.ScrollRight(4);
}

/*
00FC - SCL N
Scroll display 4 pixels to the left.
*/
void Chip8::Fun_00FC()
{
	printf("SCL\n");
	m_display.ScrollLeft(4);
}

/*
00FE
Disable extended screen mode
*/
void Chip8::Fun_00FE()
{
	m_extendedMode = false;
	m_display.SetExtendedMode(m_extendedMode);
}

/*
00FE
Enable extended screen mode
*/
void Chip8::Fun_00FF()
{
	m_extendedMode = true;
	m_display.SetExtendedMode(m_extendedMode);
}

/*
1nnn - JP addr
Jump to location nnn.

The interpreter sets the program counter to nnn.
*/
void Chip8::Fun_1nnn()
{
	printf("JP 0x%X\n", m_decoder.nnn);
	m_pc = m_decoder.nnn;
}

/*
2nnn - CALL addr
Call subroutine at nnn.

The interpreter increments the stack pointer, then puts the current PC on the top of the stack. The PC is then set to nnn.
*/
void Chip8::Fun_2nnn()
{
	printf("CALL 0x%X\n", m_decoder.nnn);
	*m_sp = m_pc;
	m_sp++;
	m_pc = m_decoder.nnn;
}

/*
3xkk - SE Vx, byte
Skip next instruction if Vx = kk.

The interpreter compares register Vx to kk, and if they are equal, increments the program counter by 2.
*/
void Chip8::Fun_3xkk()
{
	printf("SE V%X, 0x%X\n", m_decoder.x, m_decoder.kk);
	if (m_V[m_decoder.x] == m_decoder.kk)
		m_pc += 2;
}

/*
4xkk - SNE Vx, byte
Skip next instruction if Vx != kk.

The interpreter compares register Vx to kk, and if they are not equal, increments the program counter by 2.
*/
void Chip8::Fun_4xkk()
{
	printf("SNE V%X, 0x%X\n", m_decoder.x, m_decoder.kk);
	if (m_V[m_decoder.x] != m_decoder.kk)
		m_pc += 2;
}


/*
5xy0 - SE Vx, Vy
Skip next instruction if Vx = Vy.

The interpreter compares register Vx to register Vy, and if they are equal, increments the program counter by 2.
*/
void Chip8::Fun_5xy0()
{
	printf("SE V%X, V%X\n", m_decoder.x, m_decoder.y);
	if (m_V[m_decoder.x] == m_V[m_decoder.y])
		m_pc += 2;
}

/*
5xy2 - LD [I], Vx-Vy

Write registers Vx to Vy to memory pointed to by I.
*/
void Chip8::Fun_5xy2()
{
	printf("LD [I], V%X-V%X\n", m_decoder.x, m_decoder.y);

	uint8_t increment = 0;

	for (uint8_t index = m_decoder.x; index <= m_decoder.y; index++)
	{
		m_memory[m_indexRegister + increment] = m_V[index];
		increment++;
	}
}

/*
5xy3 - LD Vx-Vy, [I]

Load registers Vx to Vy from memory pointed to by I.
*/
void Chip8::Fun_5xy3()
{
	printf("LD V%X-V%X, [I]\n", m_decoder.x, m_decoder.y);

	uint8_t increment = 0;

	for (uint8_t index = m_decoder.x; index <= m_decoder.y; index++)
	{
		m_V[index] = m_memory[m_indexRegister + increment];
		increment++;
	}
}

/*
6xkk - LD Vx, byte
Set Vx = kk.

The interpreter puts the value kk into register Vx.
*/
void Chip8::Fun_6xkk()
{
	printf("LD V%X, 0x%X\n", m_decoder.x, m_decoder.kk);
	m_V[m_decoder.x] = m_decoder.kk;
}

/*
7xkk - ADD Vx, byte
Set Vx = Vx + kk.

Adds the value kk to the value of register Vx, then stores the result in Vx.
*/
void Chip8::Fun_7xkk()
{
	printf("ADD V%X, 0x%X\n", m_decoder.x, m_decoder.kk);
	m_V[m_decoder.x] += m_decoder.kk;
}

/*
8xy0 - LD Vx, Vy
Set Vx = Vy.

Stores the value of register Vy in register Vx.
*/
void Chip8::Fun_8xy0()
{
	printf("LD V%X, V%X\n", m_decoder.x, m_decoder.y);
	m_V[m_decoder.x] = m_V[m_decoder.y];
}

/*
8xy1 - OR Vx, Vy
Set Vx = Vx OR Vy.

Performs a bitwise OR on the values of Vx and Vy, then stores the result in Vx.
A bitwise OR compares the corrseponding bits from two values, and if either bit is 1, then the same bit in the result is also 1.
Otherwise, it is 0.

It also resets the flag register to zero IF VF reset quirk is on.
*/
void Chip8::Fun_8xy1()
{
	printf("OR V%X, V%X\n", m_decoder.x, m_decoder.y);
	m_V[m_decoder.x] |= m_V[m_decoder.y];

	if (m_VFreset_quirk)
		m_V[0xF] = 0;
}

/*
8xy2 - AND Vx, Vy
Set Vx = Vx AND Vy.

Performs a bitwise AND on the values of Vx and Vy, then stores the result in Vx.
A bitwise AND compares the corrseponding bits from two values, and if both bits are 1, then the same bit in the result is also 1.
Otherwise, it is 0.

It also resets the flag register to zero IF VF reset quirk is on.
*/
void Chip8::Fun_8xy2()
{
	printf("AND V%X, V%X\n", m_decoder.x, m_decoder.y);
	m_V[m_decoder.x] &= m_V[m_decoder.y];

	if (m_VFreset_quirk)
		m_V[0xF] = 0;
}

/*
8xy3 - XOR Vx, Vy
Set Vx = Vx XOR Vy.

Performs a bitwise exclusive OR on the values of Vx and Vy, then stores the result in Vx.
An exclusive OR compares the corrseponding bits from two values, and if the bits are not both the same, then the corresponding bit in the result is set to 1.
Otherwise, it is 0.

It also resets the flag register to zero IF VF reset quirk is on.
*/
void Chip8::Fun_8xy3()
{
	printf("XOR V%X, V%X\n", m_decoder.x, m_decoder.y);
	m_V[m_decoder.x] ^= m_V[m_decoder.y];

	if (m_VFreset_quirk)
		m_V[0xF] = 0;
}

/*
8xy4 - ADD Vx, Vy
Set Vx = Vx + Vy, set VF = carry.

The values of Vx and Vy are added together.
If the result is greater than 8 bits (i.e., > 255,) VF is set to 1, otherwise 0.
Only the lowest 8 bits of the result are kept, and stored in Vx.
*/
void Chip8::Fun_8xy4()
{
	printf("ADD V%X, V%X\n", m_decoder.x, m_decoder.y);
	uint16_t result = m_V[m_decoder.x] + m_V[m_decoder.y];
	m_V[m_decoder.x] = result & 0x00FF;

	m_V[0xF] = (result > 0xFF);
}

/*
8xy5 - SUB Vx, Vy
Set Vx = Vx - Vy, set VF = NOT borrow.

If Vx > Vy, then VF is set to 1, otherwise 0. Then Vy is subtracted from Vx, and the results stored in Vx.
*/
void Chip8::Fun_8xy5()
{
	printf("SUB V%X, V%X\n", m_decoder.x, m_decoder.y);
	uint16_t result = m_V[m_decoder.x] - m_V[m_decoder.y];
	uint8_t flag = (m_V[m_decoder.x] >= m_V[m_decoder.y]);

	m_V[m_decoder.x] = result & 0x00FF;
	m_V[0xF] = flag;
}

/*
WITHOUT SHIFING QUIRK INTERPRETATION:
	8xy6 - SHR Vx, Vy
	Set the value of Vy to Vx and then set Vx = Vx SHR 1.

	If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2.

SHIFTING QUIRK INTERPRETATION:
	8xy6 - SHR Vx
	Set Vx = Vx SHR 1.

	If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2.
*/
void Chip8::Fun_8xy6()
{
	if (!m_Shifting_quirk)
	{
		printf("SHR V%X, V%X\n", m_decoder.x, m_decoder.y);

		m_V[m_decoder.x] = m_V[m_decoder.y];
	}
	else
		printf("SHR V%X\n", m_decoder.x);

	uint8_t flag = ((m_V[m_decoder.x] & 0x01) == 1);
	m_V[m_decoder.x] /= 2;
	m_V[0xF] = flag;
}

/*
8xy7 - SUBN Vx, Vy
Set Vx = Vy - Vx, set VF = NOT borrow.

If Vy > Vx, then VF is set to 1, otherwise 0. Then Vx is subtracted from Vy, and the results stored in Vx.
*/
void Chip8::Fun_8xy7()
{
	printf("SUBN V%X, V%X\n", m_decoder.x, m_decoder.y);
	uint16_t result = m_V[m_decoder.y] - m_V[m_decoder.x];
	uint8_t flag = (m_V[m_decoder.y] >= m_V[m_decoder.x]);

	m_V[m_decoder.x] = result & 0x00FF;
	m_V[0xF] = flag;
}

/*
WITHOUT SHIFING QUIRK INTERPRETATION:
	8xyE - SHL Vx, Vy
	Set the value of Vy to Vx and then set Vx = Vx SHL 1.

	If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0. Then Vx is multiplied by 2.

SHIFTING QUIRK INTERPRETATION:
	8xyE - SHL Vx
	Set Vx = Vx SHL 1.

	If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0. Then Vx is multiplied by 2.
*/
void Chip8::Fun_8xyE()
{

	if (!m_Shifting_quirk)
	{
		printf("SHL V%X, V%X\n", m_decoder.x, m_decoder.y);

		m_V[m_decoder.x] = m_V[m_decoder.y];
	}
	else
		printf("SHL V%X\n", m_decoder.x);

	uint8_t flag = ((m_V[m_decoder.x] >> 7) == 1);
	m_V[m_decoder.x] *= 2;
	m_V[0xF] = flag;

}

/*
9xy0 - SNE Vx, Vy
Skip next instruction if Vx != Vy.

The values of Vx and Vy are compared, and if they are not equal, the program counter is increased by 2.

*/
void Chip8::Fun_9xy0()
{
	printf("SNE V%X, V%X\n", m_decoder.x, m_decoder.y);

	if (m_V[m_decoder.x] != m_V[m_decoder.y])
		m_pc += 2;
}

/*
Annn - LD I, addr
Set I = nnn.

The value of register I is set to nnn.
*/
void Chip8::Fun_Annn()
{
	printf("LD I, 0x%X\n", m_decoder.nnn);

	m_indexRegister = m_decoder.nnn;
}

/*
WITHOUT JUMPING QUIRK INTERPRETATION:
	Bnnn - JP V0, addr
	Jump to location nnn + V0.

	The program counter is set to nnn plus the value of V0.

JUMPING QUIRK INTERPRETATION:
	Bxnn - JP Vx, addr
	Jump to location nnn + Vx.

	The program counter is set to nnn plus the value of Vx.
*/
void Chip8::Fun_Bnnn()
{
	if (!m_Jumping_quirk)
	{
		printf("JP V0, 0x%X\n", m_decoder.nnn);

		m_pc = m_decoder.nnn + m_V[0];
	}
	else
	{
		printf("JP V%X, 0x%X\n", m_decoder.x, m_decoder.nnn);

		m_pc = m_decoder.nnn + m_V[m_decoder.x];
	}
}

/*
Cxkk - RND Vx, byte
Set Vx = random byte AND kk.

The interpreter generates a random number from 0 to 255, which is then ANDed with the value kk.
The results are stored in Vx. See instruction 8xy2 for more information on AND.
*/
void Chip8::Fun_Cxkk()
{
	printf("RND V%X, 0x%X\n", m_decoder.x, m_decoder.kk);

	uint8_t random = std::rand() % 256;

	m_V[m_decoder.x] = random & m_decoder.kk;
}

/*
Dxyn - DRW Vx, Vy, nibble
Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.

The interpreter reads n bytes from memory, starting at the address stored in I.
These bytes are then displayed as sprites on screen at coordinates (Vx, Vy). Sprites are XORed onto the existing screen.
If this causes any pixels to be erased, VF is set to 1, otherwise it is set to 0.
If the sprite is positioned so part of it is outside the coordinates of the display, it wraps around to the opposite side of the screen.
See instruction 8xy3 for more information on XOR, and section 2.4, Display, for more information on the Chip-8 screen and sprites.
*/
void Chip8::Fun_Dxyn()
{
	printf("DRW V%X, V%X, %X\n", m_decoder.x, m_decoder.y, m_decoder.n);

	uint8_t xPos = m_V[m_decoder.x];
	uint8_t yPos = m_V[m_decoder.y];

	m_V[0xF] = 0;

	if (m_decoder.n > 0)
	{
		for (int row = 0; row < m_decoder.n; row++)
		{
			uint8_t spriteRow = m_memory[m_indexRegister + row];
			for (int col = 0; col < 8; col++)
			{
				bool pixel = spriteRow & (0x80 >> col);
				bool conflict = m_display.ChangePixelValue(xPos, yPos, col, row, pixel, m_Clipping_quirk);
				if (conflict)
					m_V[0xF] = 1;
			}
		}
	}
	else // Draw a sprite of 16x16
	{
		if (m_interpreter == OCTO || m_interpreter == SCHIP && m_extendedMode)
		{
			for (int row = 0; row < 16; row++)
			{
				uint16_t spriteRow = (m_memory[m_indexRegister + row * 2] << 8) | m_memory[m_indexRegister + row * 2 + 1];
				for (int col = 0; col < 16; col++)
				{
					bool pixel = spriteRow & (0x8000 >> col);
					bool conflict = m_display.ChangePixelValue(xPos, yPos, col, row, pixel, m_Clipping_quirk);
					if (conflict)
						m_V[0xF] = 1;
				}
			}
		}
	}

	if (m_DisplayWait_quirk)
		m_instructionCount = m_ips;
}

/*
Ex9E - SKP Vx
Skip next instruction if key with the value of Vx is pressed.

Checks the keyboard, and if the key corresponding to the value of Vx is currently in the down position, PC is increased by 2.
*/
void Chip8::Fun_Ex9E()
{
	printf("SKP V%X\n", m_decoder.x);

	if (m_keyboard.ReadKeyboard(m_V[m_decoder.x]))
		m_pc += 2;
}

/*
ExA1 - SKNP Vx
Skip next instruction if key with the value of Vx is not pressed.

Checks the keyboard, and if the key corresponding to the value of Vx is currently in the up position, PC is increased by 2.
*/
void Chip8::Fun_ExA1()
{
	printf("SKNP V%X\n", m_decoder.x);

	if (!m_keyboard.ReadKeyboard(m_V[m_decoder.x]))
		m_pc += 2;
}

/*
ExA1 - DW #F000, #NNNN

Assign next 16 bit word to I, and set PC behind it, this is a four byte instruction.
*/
void Chip8::Fun_F000()
{
	uint16_t word = Fetch(); // Get next 2 bytes to get the value of I and increment pc

	printf("DW F000 %.4X\n", word);

	m_indexRegister = word;

	// PC alredy incremented
}

/*
Fx07 - LD Vx, DT
Set Vx = delay timer value.

The value of DT is placed into Vx.
*/
void Chip8::Fun_Fx07()
{
	printf("LD V%X, DT\n", m_decoder.x);

	m_V[m_decoder.x] = m_delayTimer;
}

/*
Fx0A - LD Vx, K
Wait for a key press, store the value of the key in Vx.

All execution stops until a key is pressed, then the value of that key is stored in Vx.
*/
void Chip8::Fun_Fx0A()
{
	//printf("LD V%X, K (Waiting for key press...)\n", m_decoder.x);

	int key = -1;
	if (m_keyboard.WaitForInput(&key))
	{
		m_V[m_decoder.x] = key;
		printf("LD V%X, K (KeyPressed)\n", m_decoder.x);
	}
	else
		m_pc -= 2;
}

/*
Fx15 - LD DT, Vx
Set delay timer = Vx.

DT is set equal to the value of Vx.
*/
void Chip8::Fun_Fx15()
{
	printf("LD DT, V%X\n", m_decoder.x);

	m_delayTimer = m_V[m_decoder.x];
}

/*3
Fx18 - LD ST, Vx
Set sound timer = Vx.

ST is set equal to the value of Vx.
*/
void Chip8::Fun_Fx18()
{
	printf("LD ST, V%X\n", m_decoder.x);

	m_soundTimer = m_V[m_decoder.x];
}

/*
Fx1E - ADD I, Vx
Set I = I + Vx.

The values of I and Vx are added, and the results are stored in I.
*/
void Chip8::Fun_Fx1E()
{
	printf("ADD I, V%X\n", m_decoder.x);

	m_indexRegister += m_V[m_decoder.x];
}

/*
Fx29 - LD F, Vx
LORES FONT: Set I = location of sprite for digit Vx.

The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx.
*/
void Chip8::Fun_Fx29()
{
	printf("LD F, V%X\n", m_decoder.x);

	m_indexRegister = m_V[m_decoder.x] * 5; // The font sprites have a length of 5 bytes.
}

/*
Fx30 - LD F, Vx
HIRES FONT: Set I = location of sprite for digit Vx.

The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx.
*/
void Chip8::Fun_Fx30()
{
	printf("LD F, V%X\n", m_decoder.x);

	m_indexRegister = m_V[m_decoder.x] * 10; // The font sprites have a length of 10 bytes.
}

/*
Fx33 - LD B, Vx
Store BCD representation of Vx in memory locations I, I+1, and I+2.

The interpreter takes the decimal value of Vx, and places the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.
*/
void Chip8::Fun_Fx33()
{
	printf("LD B, V%X\n", m_decoder.x);

	uint8_t hundreds = m_V[m_decoder.x] / 100;
	uint8_t tens = (m_V[m_decoder.x] % 100) / 10;
	uint8_t ones = (m_V[m_decoder.x] % 100) % 10;

	m_memory[m_indexRegister] = hundreds;
	m_memory[m_indexRegister + 1] = tens;
	m_memory[m_indexRegister + 2] = ones;
}

/*
MEMORY QUIRK INTERPRETATION:
	Fx55 - LD [I], Vx
	Store registers V0 through Vx in memory starting at location I.

	The interpreter copies the values of registers V0 through Vx into memory, starting at the address in I.

	The index register (I) is incremented.

WITHOUT MEMORY QUIRK INTERPRETATION:
	Fx55 - LD [I], Vx
	Store registers V0 through Vx in memory starting at location I.

	The interpreter copies the values of registers V0 through Vx into memory, starting at the address in I.

	The index register (I) is NOT incremented.
*/
void Chip8::Fun_Fx55()
{
	if (m_Memory_quirk)
	{
		printf("LD [I], V%X\n", m_decoder.x);

		for (uint8_t i = 0; i <= m_decoder.x; i++)
			m_memory[m_indexRegister++] = m_V[i];
	}
	else
	{
		printf("LD [I], V%X\n", m_decoder.x);

		for (uint8_t i = 0; i <= m_decoder.x; i++)
			m_memory[m_indexRegister + i] = m_V[i];
	}
}

/*
MEMORY QUIRK INTERPRETATION:
	Fx65 - LD Vx, [I]
	Read registers V0 through Vx from memory starting at location I.

	The interpreter reads values from memory starting at location I into registers V0 through Vx.

	The index register (I) is incremented.

WITHOUT MEMORY QUIRK INTERPRETATION:
	Fx65 - LD Vx, [I]
	Read registers V0 through Vx from memory starting at location I.

	The interpreter reads values from memory starting at location I into registers V0 through Vx.

	The index register (I) is NOT incremented.
*/
void Chip8::Fun_Fx65()
{
	if (m_Memory_quirk)
	{
		printf("LD V%X, [I]\n", m_decoder.x);

		for (uint8_t i = 0; i <= m_decoder.x; i++)
			m_V[i] = m_memory[m_indexRegister++];
	}
	else
	{
		printf("LD V%X, [I]\n", m_decoder.x);

		for (uint8_t i = 0; i <= m_decoder.x; i++)
			m_V[i] = m_memory[m_indexRegister + i];
	}
}

/*
	Fx75 - LD R, VX
	Store V0 through VX to HP-48 RPL user flags (X <= 7).
*/
void Chip8::Fun_Fx75()
{

}

/*
	Fx85 - LD VX, R
	Read V0 through VX to HP-48 RPL user flags (X <= 7).
*/
void Chip8::Fun_Fx85()
{
}
