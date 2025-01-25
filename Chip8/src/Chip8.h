#pragma once
#include <stdint.h>
#include <fstream>
#include <string>
#include <array>
#include <cstdlib>
#include <iostream>
#include <chrono>
#include <thread>

#include "Display.h"
#include "Keyboard.h"
#include "Configuration.h"

class Chip8
{
public:

	Chip8();
	
	void LoadGame(const std::string& gameName);

	void Simulate_Cycle();

	void Run();

private:

	void LoadFontSet();

	void InitQuirks();
	void UpdateQuirk(bool& quirkMember, const int configValue);

	uint16_t Fetch();
	std::function<void()> Decode(uint16_t opcode);

	void Fun_00Cn(); // SCD N
	void Fun_00Dn(); // SCD U
	void Fun_00E0(); // CLS
	void Fun_00EE(); // RET
	void Fun_00FB(); // SCR
	void Fun_00FC(); // SCL
	void Fun_00FE(); // LOW
	void Fun_00FF(); // HIGH
	void Fun_1nnn(); // JP addr
	void Fun_2nnn(); // CALL addr
	void Fun_3xkk(); // SE Vx, byte
	void Fun_4xkk(); // SNE Vx, byte
	void Fun_5xy0(); // SE Vx, Vy
	void Fun_5xy2(); // LD [I], Vx-Vy
	void Fun_5xy3(); // LD Vx-Vy, [I]
	void Fun_6xkk(); // LD Vx, byte
	void Fun_7xkk(); // ADD Vx, byte
	void Fun_8xy0(); // LD Vx, Vy
	void Fun_8xy1(); // OR Vx, Vy
	void Fun_8xy2(); // AND Vx, Vy
	void Fun_8xy3(); // XOR Vx, Vy
	void Fun_8xy4(); // ADD Vx, Vy
	void Fun_8xy5(); // SUB Vx, Vy
	void Fun_8xy6(); // SHR Vx {, Vy}
	void Fun_8xy7(); // SUBN Vx, Vy
	void Fun_8xyE(); // SHL Vx {, Vy}
	void Fun_9xy0(); // SNE Vx, Vy
	void Fun_Annn(); // LD I, addr
	void Fun_Bnnn(); // JP V0, addr
	void Fun_Cxkk(); // RND Vx, byte
	void Fun_Dxyn(); // DRW Vx, Vy, nibble
	void Fun_Ex9E(); // SKP Vx
	void Fun_ExA1(); // SKNP Vx
	void Fun_F000(); // DW #F000, #NNNN
	void Fun_Fx01(); // DW #Fx01
	void Fun_Fx07(); // LD Vx, DT
	void Fun_Fx0A(); // LD Vx, K
	void Fun_Fx15(); // LD DT, Vx
	void Fun_Fx18(); // LD ST, Vx
	void Fun_Fx1E(); // ADD I, Vx
	void Fun_Fx29(); // LD F, Vx
	void Fun_Fx30(); // LD F, Vx
	void Fun_Fx33(); // LD B, Vx
	void Fun_Fx55(); // LD [I], Vx
	void Fun_Fx65(); // LD Vx, [I]
	void Fun_Fx75(); // LD R, VX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
	void Fun_Fx85(); // LD VX, R XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
	
private:
	std::array<uint8_t, 16> m_V;
	std::array<uint8_t, 0x10000> m_memory;


	uint16_t m_indexRegister; // Only the lowest 12 bits are used. Points to locations in the memory (0x000 - 0xFFF)
	uint16_t m_pc; // Program Counter. Stores the currently executing address.

	std::array<uint16_t, 16> m_stack;
	std::array<uint16_t, 16>::iterator m_sp;

	uint8_t m_delayTimer;
	uint8_t m_soundTimer;

	Configuration m_config;	
	Display m_display;
	Keyboard m_keyboard;

	struct OPCODE_DECO
	{
		uint8_t type;
		uint16_t nnn;	//A 12-bit value, the lowest 12 bits of the instruction
		uint8_t n;		//A 4-bit value, the lowest 4 bits of the instruction
		uint8_t x;		//A 4-bit value, the lower 4 bits of the high byte of the instruction
		uint8_t y;		//A 4-bit value, the upper 4 bits of the low byte of the instruction
		uint8_t kk;		//An 8-bit value, the lowest 8 bits of the instruction
	} m_decoder;

	enum INTERPRETER
	{
		VIP = 0,
		SCHIP,
		OCTO,
		XOCHIP
	} m_interpreter;

	bool m_VFreset_quirk;
	bool m_Memory_quirk;
	bool m_DisplayWait_quirk;
	bool m_Clipping_quirk;
	bool m_Shifting_quirk;
	bool m_Jumping_quirk;

	unsigned int m_ipf;
	unsigned int m_instructionCount;
};

