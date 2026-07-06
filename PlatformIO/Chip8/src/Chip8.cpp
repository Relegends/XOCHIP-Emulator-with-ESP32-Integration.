#include "Chip8.h"
#include "RomSelector.h"

Chip8::Chip8() :
	m_V({ 0 }),
	m_pc(0x200), // Start Address
	m_memory({ 0 }),
	m_stack({ 0 }),
	m_sp(m_stack.begin()),
	m_config(),
	m_display(),
	m_audio(),
	m_keyboard(),
	m_network(),
	m_decoder({ 0 }),
	m_interpreter(INTERPRETER::XOCHIP),
	m_instructionCount(0),
	m_ipf(0),
	m_logggingMode(LOGGINGMODE::NONE)
{
	
}

bool Chip8::Init(const std::string& configFilePath)
{
	LOG("{Chip8} Initializing Chip8...");
	if(!m_config.Init(configFilePath)) return false;
	if(!m_display.Init(
		m_config.GetWindowWidth(), m_config.GetWindowHeight(),
		m_config.GetDisplayWidth(), m_config.GetDisplayHeight(),
		m_config.GetDisplayBackgroundColor(), m_config.GetDisplayForeground_1Color(),
		m_config.GetDisplayForeground_2Color(), m_config.GetDisplayBlendedColor(),
		m_config.GetPanelWidth(), m_config.GetPanelHeight(),
		m_config.GetRotation(), m_config.GetRGBOrder(), m_config.GetIsFlipY())) return false;
	if(!m_audio.Init(m_config.GetAudioFrequency(), m_config.GetAudioSampleRate(), m_config.GetAudioVolume())) return false;
	if(!m_network.Init(m_config.GetNetworkDeviceName(), m_config.GetNetworkWifiChannel())) return false;
	if(!m_keyboard.Init(m_config.GetInputMap())) return false;

	m_V = { 0 };
	m_memory = { 0 };
	m_pc = 0x200; // Start Address
	m_stack = { 0 };
	m_sp = m_stack.begin();
	m_decoder = { 0 };
	m_instructionCount = 0;
	m_delayTimer = 0;
	m_soundTimer = 0;

	m_interpreter = (Chip8::INTERPRETER)m_config.GetInterpretedMode();
	m_logggingMode = (Chip8::LOGGINGMODE)m_config.GetLoggingMode();
	m_ipf = m_config.GetIPF();

	std::srand(static_cast<unsigned int>(std::time(0)));

	InitQuirks();
	LoadFontSet();

	return true;
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
			0xF0, 0x80, 0xF0, 0x80, 0x80  // F
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
			0xFF, 0xFF, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xFF, 0xFF, // 0
			0x18, 0x78, 0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0xFF, 0xFF, // 1
			0xFF, 0xFF, 0x03, 0x03, 0xFF, 0xFF, 0xC0, 0xC0, 0xFF, 0xFF, // 2
			0xFF, 0xFF, 0x03, 0x03, 0xFF, 0xFF, 0x03, 0x03, 0xFF, 0xFF, // 3
			0xC3, 0xC3, 0xC3, 0xC3, 0xFF, 0xFF, 0x03, 0x03, 0x03, 0x03, // 4
			0xFF, 0xFF, 0xC0, 0xC0, 0xFF, 0xFF, 0x03, 0x03, 0xFF, 0xFF, // 5
			0xFF, 0xFF, 0xC0, 0xC0, 0xFF, 0xFF, 0xC3, 0xC3, 0xFF, 0xFF, // 6
			0xFF, 0xFF, 0x03, 0x03, 0x06, 0x0C, 0x18, 0x18, 0x18, 0x18, // 7
			0xFF, 0xFF, 0xC3, 0xC3, 0xFF, 0xFF, 0xC3, 0xC3, 0xFF, 0xFF, // 8
			0xFF, 0xFF, 0xC3, 0xC3, 0xFF, 0xFF, 0x03, 0x03, 0xFF, 0xFF, // 9
			0x7E, 0xFF, 0xC3, 0xC3, 0xC3, 0xFF, 0xFF, 0xC3, 0xC3, 0xC3, // A
			0xFC, 0xFC, 0xC3, 0xC3, 0xFC, 0xFC, 0xC3, 0xC3, 0xFC, 0xFC, // B
			0x3C, 0xFF, 0xC3, 0xC0, 0xC0, 0xC0, 0xC0, 0xC3, 0xFF, 0x3C, // C
			0xFC, 0xFE, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xFE, 0xFC, // D
			0xFF, 0xFF, 0xC0, 0xC0, 0xFF, 0xFF, 0xC0, 0xC0, 0xFF, 0xFF, // E
			0xFF, 0xFF, 0xC0, 0xC0, 0xFF, 0xFF, 0xC0, 0xC0, 0xC0, 0xC0  // F
		};
		break;
	case Chip8::XOCHIP:
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
			0xFF, 0xFF, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xFF, 0xFF, // 0
			0x18, 0x78, 0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0xFF, 0xFF, // 1
			0xFF, 0xFF, 0x03, 0x03, 0xFF, 0xFF, 0xC0, 0xC0, 0xFF, 0xFF, // 2
			0xFF, 0xFF, 0x03, 0x03, 0xFF, 0xFF, 0x03, 0x03, 0xFF, 0xFF, // 3
			0xC3, 0xC3, 0xC3, 0xC3, 0xFF, 0xFF, 0x03, 0x03, 0x03, 0x03, // 4
			0xFF, 0xFF, 0xC0, 0xC0, 0xFF, 0xFF, 0x03, 0x03, 0xFF, 0xFF, // 5
			0xFF, 0xFF, 0xC0, 0xC0, 0xFF, 0xFF, 0xC3, 0xC3, 0xFF, 0xFF, // 6
			0xFF, 0xFF, 0x03, 0x03, 0x06, 0x0C, 0x18, 0x18, 0x18, 0x18, // 7
			0xFF, 0xFF, 0xC3, 0xC3, 0xFF, 0xFF, 0xC3, 0xC3, 0xFF, 0xFF, // 8
			0xFF, 0xFF, 0xC3, 0xC3, 0xFF, 0xFF, 0x03, 0x03, 0xFF, 0xFF, // 9
			0x7E, 0xFF, 0xC3, 0xC3, 0xC3, 0xFF, 0xFF, 0xC3, 0xC3, 0xC3, // A
			0xFC, 0xFC, 0xC3, 0xC3, 0xFC, 0xFC, 0xC3, 0xC3, 0xFC, 0xFC, // B
			0x3C, 0xFF, 0xC3, 0xC0, 0xC0, 0xC0, 0xC0, 0xC3, 0xFF, 0x3C, // C
			0xFC, 0xFE, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xFE, 0xFC, // D
			0xFF, 0xFF, 0xC0, 0xC0, 0xFF, 0xFF, 0xC0, 0xC0, 0xFF, 0xFF, // E
			0xFF, 0xFF, 0xC0, 0xC0, 0xFF, 0xFF, 0xC0, 0xC0, 0xC0, 0xC0  // F
		};
		break;
	default:
		LOGE("{Chip8} NOT VALID INTERPRETER!!!");
		abort();
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
		LOGE("{Chip8} NOT VALID INTERPRETER!!!");
		abort();
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

void Chip8::Decode(uint16_t opcode)
{
	m_decoder.type = (opcode >> 12) & 0x000F;
	m_decoder.nnn = opcode & 0x0FFF;
	m_decoder.kk = opcode & 0x00FF;
	m_decoder.n = opcode & 0x000F;
	m_decoder.x = (opcode >> 8) & 0x000F;
	m_decoder.y = (opcode >> 4) & 0x000F;

	if (m_decoder.kk != 0x0a && m_decoder.kk != 0xa8 && m_logggingMode != LOGGINGMODE::NONE)
	{
		LOG("I = 0x%.4X ", m_indexRegister);
		for (uint8_t i = 0; i < m_V.size(); i++)
			LOG("V%.1X = 0x%.2X ", i, m_V[i]);
		LOG("PC: % .4X OPCODE: % .4X -> ", m_pc - 2, opcode);
	}

	switch (m_decoder.type)
	{
	case 0x0:
		switch (m_decoder.y)
		{
		case 0xC: return Fun_00Cn();
		case 0xD: return Fun_00Dn();
		}
		switch (m_decoder.kk)
		{
		case 0xE0: return Fun_00E0();
		case 0xEE: return Fun_00EE();
		case 0xFB: return Fun_00FB();
		case 0xFC: return Fun_00FC();
		case 0xFE: return Fun_00FE();
		case 0xFF: return Fun_00FF();
		}
		break;
	case 0x1: return Fun_1nnn();
	case 0x2: return Fun_2nnn();
	case 0x3: return Fun_3xkk();
	case 0x4: return Fun_4xkk();
	case 0x5:
		switch (m_decoder.n)
		{
		case 0x0: return Fun_5xy0();
		case 0x2: return Fun_5xy2();
		case 0x3: return Fun_5xy3();
		}
		break;
	case 0x6: return Fun_6xkk();
	case 0x7: return Fun_7xkk();
	case 0x8:
		switch (m_decoder.n)
		{
		case 0x0: return Fun_8xy0();
		case 0x1: return Fun_8xy1();
		case 0x2: return Fun_8xy2();
		case 0x3: return Fun_8xy3();
		case 0x4: return Fun_8xy4();
		case 0x5: return Fun_8xy5();
		case 0x6: return Fun_8xy6();
		case 0x7: return Fun_8xy7();
		case 0xE: return Fun_8xyE();
		}
		break;
	case 0x9: return Fun_9xy0();
	case 0xA: return Fun_Annn();
	case 0xB: return Fun_Bnnn();
	case 0xC: return Fun_Cxkk();
	case 0xD: return Fun_Dxyn();
	case 0xE:
		switch (m_decoder.kk)
		{
		case 0x9E: return Fun_Ex9E();
		case 0xA1: return Fun_ExA1();
		}
		break;
	case 0xF:
		switch (m_decoder.kk)
		{
		case 0x00: return Fun_F000();
		case 0x01: return Fun_Fx01();
		case 0x02: return Fun_F002();
		case 0x07: return Fun_Fx07();
		case 0x0A: return Fun_Fx0A();
		case 0x15: return Fun_Fx15();
		case 0x18: return Fun_Fx18();
		case 0x1E: return Fun_Fx1E();
		case 0x29: return Fun_Fx29();
		case 0x30: return Fun_Fx30();
		case 0x33: return Fun_Fx33();
		case 0x3A: return Fun_Fx3A();
		case 0x55: return Fun_Fx55();
		case 0x65: return Fun_Fx65();
		case 0x75: return Fun_Fx75();
		case 0x85: return Fun_Fx85();
		case 0xA0: return Fun_FxA0();
		case 0xA1: return Fun_FxA1();
		case 0xA2: return Fun_FxA2();
		case 0xA3: return Fun_FxA3();
		case 0xA4: return Fun_FxA4();
		case 0xA5: return Fun_FxA5();
		case 0xA6: return Fun_FxA6();
		case 0xA7: return Fun_FxA7();
		case 0xA8: return Fun_FxA8();
		}
		break;
	}

	printf("ERROR: Decoded opcode not implemented! OPCODE: %.4X\n", opcode);
	abort();
}

void Chip8::SkipNextInstruction()
{
	if (m_interpreter != XOCHIP)
		m_pc += 2;
	else
	{
		uint16_t opcode = Fetch();
		m_pc -= 2; //Fetch increments m_pc so it needs to get reset before updating it
		if (opcode == 0XF000)
			m_pc += 4;
		else
			m_pc += 2;
	}
}

bool Chip8::LoadGame(const std::string& gameName)
{
	std::ifstream gameFile(gameName, std::ifstream::binary);

	if (!gameFile.is_open()) {
		LOGE("{Chip8} Failed to open game file: %s", gameName.c_str());
		return false;
	}
	gameFile.seekg(0, gameFile.end);
	std::streamoff fileSize = gameFile.tellg();
	gameFile.seekg(0, gameFile.beg);

	std::vector<char> buffer(fileSize);

	gameFile.read(buffer.data(), fileSize);

	gameFile.close();

	for (int i = 0; i < fileSize; i++) {
		m_memory[m_pc++] = buffer[i];
	}

	m_pc = 0x200; // Return the pc to the start of the program. 

	LOG("{Chip8} Game loaded: %s", gameName.c_str());
	return true;
}

std::string Chip8::SelectRom(const std::string& romsDir)
{
	RomSelector selector;
	return selector.Select(romsDir, m_display.GetPanel(), &m_keyboard, m_config.GetIsFlipY(), m_config.GetIsFlipX());
}

void Chip8::Simulate_Cycle()
{
	Chip8::Decode(Chip8::Fetch());
}

void Chip8::Run()
{
	LOG("{Chip8} Starting Run loop...");
	const float targetFPS = 60.0f;
	const float targetTimeBtwFrames = 1000.0f / targetFPS;
	const TickType_t framePeriod = static_cast<TickType_t>(targetTimeBtwFrames / portTICK_PERIOD_MS);
	TickType_t lastFrameTick = xTaskGetTickCount();
	const int64_t frameBudgetUs = 1000000LL / 60; // 16 667 µs
	unsigned int displayFrame = 0;

	while (!m_display.IsWindowClose())
	{
		m_keyboard.ProcessInput();
		int64_t frameStart   = esp_timer_get_time();
		int64_t frameDeadline = frameStart + frameBudgetUs;

		int64_t displayUs = 0;
		if (++displayFrame >= 2)
		{
			int64_t t0 = esp_timer_get_time();
			m_display.Update();
			displayUs = esp_timer_get_time() - t0;
			displayFrame = 0;
		}

		m_instructionCount = 0;
		while (esp_timer_get_time() < frameDeadline)
		{
			Simulate_Cycle();
			m_instructionCount++;
		}

		m_keyboard.SavePreviousInputs();

		//LOG("{Chip8} INS: %d  DISP: %lldus  FRAME: %lldus", m_instructionCount, displayUs, esp_timer_get_time() - frameStart);
		UpdateTimers();
		vTaskDelayUntil(&lastFrameTick, framePeriod);
	}
	m_audio.Close();
}

void Chip8::UpdateTimers()
{
	if (m_delayTimer > 0)
		m_delayTimer--;

	if (m_soundTimer > 0)
	{
		m_soundTimer--;
		m_audio.GenerateAudioSamples();
		m_audio.PlaySound(true);
	}
	else
		m_audio.PlaySound(false);
}

/*
00CN - SCD N
Scroll display N lines down.
*/
void Chip8::Fun_00Cn()
{
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("SCD %d\n", m_decoder.n); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("scroll-down %d\n", m_decoder.n); break;
	}

	m_display.ScrollDown(m_decoder.n);
}

/*
00CN - SCU N
Scroll display N lines up.
*/
void Chip8::Fun_00Dn()
{
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("SCU %d\n", m_decoder.n); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("scroll-up %d\n", m_decoder.n); break;
	}

	m_display.ScrollUp(m_decoder.n);
}

/*
00E0 - CLS
Clear the display.
*/
void Chip8::Fun_00E0()
{
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("CLS\n"); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("clear\n"); break;
	}

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
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("RET\n"); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("return\n"); break;
	}

	m_sp--;
	m_pc = *m_sp;
}

/*
00FB - SCROLL R N
Scroll display 4 pixels to the right.
*/
void Chip8::Fun_00FB()
{
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("SCR\n"); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("scroll-right\n"); break;
	}

	m_display.ScrollRight(4);
}

/*
00FC - SCL N
Scroll display 4 pixels to the left.
*/
void Chip8::Fun_00FC()
{
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("SCL\n"); break;
	case Chip8::LOGGINGMODE::OCTO_LOG:  LOG("scroll-left\n"); break;
	}

	m_display.ScrollLeft(4);
}

/*
00FE
Disable extended screen mode
*/
void Chip8::Fun_00FE()
{
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("LOW\n"); break;
	case Chip8::LOGGINGMODE::OCTO_LOG:  LOG("lores\n"); break;
	}

	m_display.SetExtendedMode(false);
}

/*
00FE
Enable extended screen mode
*/
void Chip8::Fun_00FF()
{
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("HIGH\n"); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("hires\n"); break;
	}

	m_display.SetExtendedMode(true);
}

/*
1nnn - JP addr
Jump to location nnn.

The interpreter sets the program counter to nnn.
*/
void Chip8::Fun_1nnn()
{
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("JP 0x%X\n", m_decoder.nnn); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("jump 0x%X\n", m_decoder.nnn); break;
	}

	m_pc = m_decoder.nnn;
}

/*
2nnn - CALL addr
Call subroutine at nnn.

The interpreter increments the stack pointer, then puts the current PC on the top of the stack. The PC is then set to nnn.
*/
void Chip8::Fun_2nnn()
{
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("CALL 0x%X\n", m_decoder.nnn); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG(":call 0x%X\n", m_decoder.nnn); break;
	}

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
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("SE V%X, 0x%X\n", m_decoder.x, m_decoder.kk); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("if v%X != %d then\n", m_decoder.x, m_decoder.kk); break;
	}

	if (m_V[m_decoder.x] == m_decoder.kk)
		SkipNextInstruction();
}

/*
4xkk - SNE Vx, byte
Skip next instruction if Vx != kk.

The interpreter compares register Vx to kk, and if they are not equal, increments the program counter by 2.
*/
void Chip8::Fun_4xkk()
{
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("SNE V%X, 0x%X\n", m_decoder.x, m_decoder.kk); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("if v%X == %d then\n", m_decoder.x, m_decoder.kk); break;
	}

	if (m_V[m_decoder.x] != m_decoder.kk)
		SkipNextInstruction();
}


/*
5xy0 - SE Vx, Vy
Skip next instruction if Vx = Vy.

The interpreter compares register Vx to register Vy, and if they are equal, increments the program counter by 2.
*/
void Chip8::Fun_5xy0()
{
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("SE V%X, V%X\n", m_decoder.x, m_decoder.y); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("if v%X != v%X then\n", m_decoder.x, m_decoder.y); break;
	}

	if (m_V[m_decoder.x] == m_V[m_decoder.y])
		SkipNextInstruction();
}

/*
5xy2 - LD [I], Vx-Vy

Write registers Vx to Vy to memory pointed to by I.
*/
void Chip8::Fun_5xy2()
{
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("LD [I], V%X-V%X\n", m_decoder.x, m_decoder.y); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("save v%X - v%X\n", m_decoder.x, m_decoder.y); break;
	}

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
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("LD V%X-V%X, [I]\n", m_decoder.x, m_decoder.y); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("load v%X - v%X\n", m_decoder.x, m_decoder.y); break;
	}

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
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("LD V%X, 0x%X\n", m_decoder.x, m_decoder.kk); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("v%X := %d\n", m_decoder.x, m_decoder.kk); break;
	}

	m_V[m_decoder.x] = m_decoder.kk;
}

/*
7xkk - ADD Vx, byte
Set Vx = Vx + kk.

Adds the value kk to the value of register Vx, then stores the result in Vx.
*/
void Chip8::Fun_7xkk()
{
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("ADD V%X, 0x%X\n", m_decoder.x, m_decoder.kk); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("v%X += %d\n", m_decoder.x, m_decoder.kk); break;
	}

	m_V[m_decoder.x] += m_decoder.kk;
}

/*
8xy0 - LD Vx, Vy
Set Vx = Vy.

Stores the value of register Vy in register Vx.
*/
void Chip8::Fun_8xy0()
{
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("LD V%X, V%X\n", m_decoder.x, m_decoder.y); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("v%X := v%X\n", m_decoder.x, m_decoder.y); break;
	}

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
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("OR V%X, V%X\n", m_decoder.x, m_decoder.y); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("v%X |= v%X\n", m_decoder.x, m_decoder.y); break;
	}

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
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("AND V%X, V%X\n", m_decoder.x, m_decoder.y); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("v%X &= v%X\n", m_decoder.x, m_decoder.y); break;
	}

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
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("XOR V%X, V%X\n", m_decoder.x, m_decoder.y); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("v%X ^= v%X\n", m_decoder.x, m_decoder.y); break;
	}

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
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("ADD V%X, V%X\n", m_decoder.x, m_decoder.y); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("v%X += v%X\n", m_decoder.x, m_decoder.y); break;
	}

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
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("SUB V%X, V%X\n", m_decoder.x, m_decoder.y); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("v%X -= v%X\n", m_decoder.x, m_decoder.y); break;
	}

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
		switch (m_logggingMode)
		{
		case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("SHR V%X, V%X\n", m_decoder.x, m_decoder.y); break;
		case Chip8::LOGGINGMODE::OCTO_LOG: LOG("v%X >>= v%X\n", m_decoder.x, m_decoder.y); break;
		}

		m_V[m_decoder.x] = m_V[m_decoder.y];
	}
	else
	{
		switch (m_logggingMode)
		{
		case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("SHR V%X\n", m_decoder.x); break;
		case Chip8::LOGGINGMODE::OCTO_LOG: LOG("v%X >>= v%X\n", m_decoder.x, m_decoder.y); break;
		}
	}

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
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("SUBN V%X, V%X\n", m_decoder.x, m_decoder.y); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("v%X =- v%X\n", m_decoder.x, m_decoder.y); break;
	}

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
		switch (m_logggingMode)
		{
		case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("SHL V%X, V%X\n", m_decoder.x, m_decoder.y); break;
		case Chip8::LOGGINGMODE::OCTO_LOG: LOG("v%X <<= v%X\n", m_decoder.x, m_decoder.y); break;
		}

		m_V[m_decoder.x] = m_V[m_decoder.y];
	}
	else
	{
		switch (m_logggingMode)
		{
		case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("SHL V%X\n", m_decoder.x); break;
		case Chip8::LOGGINGMODE::OCTO_LOG: LOG("v%X <<= v%X\n", m_decoder.x, m_decoder.y); break;
		}
	}

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
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("SNE V%X, V%X\n", m_decoder.x, m_decoder.y); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("if v%X == v%X then\n", m_decoder.x, m_decoder.y); break;
	}

	if (m_V[m_decoder.x] != m_V[m_decoder.y])
		SkipNextInstruction();
}

/*
Annn - LD I, addr
Set I = nnn.

The value of register I is set to nnn.
*/
void Chip8::Fun_Annn()
{
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("LD I, 0x%X\n", m_decoder.nnn); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("i := 0x%X\n", m_decoder.nnn); break;
	}

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
		switch (m_logggingMode)
		{
		case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("JP V0, 0x%X\n", m_decoder.nnn); break;
		case Chip8::LOGGINGMODE::OCTO_LOG: LOG("jump0 0x%X\n", m_decoder.nnn); break;
		}

		m_pc = m_decoder.nnn + m_V[0];
	}
	else
	{
		switch (m_logggingMode)
		{
		case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("JP V%X, 0x%X\n", m_decoder.x, m_decoder.nnn); break;
		case Chip8::LOGGINGMODE::OCTO_LOG: LOG("jump0 0x%X + v%X\n", m_decoder.nnn, m_decoder.x); break;
		}

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
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("RND V%X, 0x%X\n", m_decoder.x, m_decoder.kk); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("v%X := random %d\n", m_decoder.x, m_decoder.kk); break;
	}

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
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("DRW V%X, V%X, %X\n", m_decoder.x, m_decoder.y, m_decoder.n); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("sprite v%X, v%X, %d\n", m_decoder.x, m_decoder.y, m_decoder.n); break;
	}

	uint8_t xPos = m_V[m_decoder.x];
	uint8_t yPos = m_V[m_decoder.y];

	m_V[0xF] = 0;

	if (m_decoder.n > 0)
	{
		if (m_display.GetActivePlane() != Display::ACTIVEPLANE::BOTH)
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
		else
		{
			for (int row = 0; row < m_decoder.n; row++)
			{
				uint8_t spriteRowLSB = m_memory[m_indexRegister + row];
				uint8_t spriteRowMSB = m_memory[m_indexRegister + row + m_decoder.n];
				for (int col = 0; col < 8; col++)
				{
					bool pixel = spriteRowLSB & (0x80 >> col);
					bool conflictLSB = m_display.ChangePixelValue(xPos, yPos, col, row, pixel, m_Clipping_quirk, Display::ACTIVEPLANE::LSB_PLANE);
					pixel = spriteRowMSB & (0x80 >> col);
					bool conflictMSB = m_display.ChangePixelValue(xPos, yPos, col, row, pixel, m_Clipping_quirk, Display::ACTIVEPLANE::MSB_PLANE);
					if (conflictLSB || conflictMSB)
						m_V[0xF] = 1;
				}
			}
		}
	}
	else // Draw a sprite of 16x16
	{
		if ((m_interpreter == SCHIP && m_display.GetExtendedMode()) || m_interpreter == OCTO ||
			(m_interpreter == XOCHIP && m_display.GetActivePlane() != Display::ACTIVEPLANE::BOTH))
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
		else if (m_interpreter == XOCHIP && m_display.GetActivePlane() == Display::ACTIVEPLANE::BOTH)
		{
			for (int row = 0; row < 16; row++)
			{
				uint16_t spriteRowLSB = (m_memory[m_indexRegister + row * 2] << 8) | m_memory[m_indexRegister + row * 2 + 1];
				uint16_t spriteRowMSB = (m_memory[m_indexRegister + (row + 16) * 2] << 8) | m_memory[m_indexRegister + (row + 16) * 2 + 1];
				for (int col = 0; col < 16; col++)
				{
					bool pixel = spriteRowLSB & (0x80 >> col);
					bool conflictLSB = m_display.ChangePixelValue(xPos, yPos, col, row, pixel, m_Clipping_quirk, Display::ACTIVEPLANE::LSB_PLANE);
					pixel = spriteRowMSB & (0x80 >> col);
					bool conflictMSB = m_display.ChangePixelValue(xPos, yPos, col, row, pixel, m_Clipping_quirk, Display::ACTIVEPLANE::MSB_PLANE);
					if (conflictLSB || conflictMSB)
						m_V[0xF] = 1;
				}
			}
		}
	}

	if (m_DisplayWait_quirk)
		m_instructionCount = m_ipf;
}

/*
Ex9E - SKP Vx
Skip next instruction if key with the value of Vx is pressed.

Checks the keyboard, and if the key corresponding to the value of Vx is currently in the down position, PC is increased by 2.
*/
void Chip8::Fun_Ex9E()
{
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("SKP V%X\n", m_decoder.x); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("if v%X -key then\n", m_decoder.x); break;
	}

	if (m_keyboard.ReadKeyboard(m_V[m_decoder.x]))
		SkipNextInstruction();
}

/*
ExA1 - SKNP Vx
Skip next instruction if key with the value of Vx is not pressed.

Checks the keyboard, and if the key corresponding to the value of Vx is currently in the up position, PC is increased by 2.
*/
void Chip8::Fun_ExA1()
{
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("SKNP V%X\n", m_decoder.x); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("if v%X key then\n", m_decoder.x); break;
	}

	if (!m_keyboard.ReadKeyboard(m_V[m_decoder.x]))
		SkipNextInstruction();
}

/*
ExA1 - DW #F000, #NNNN

Assign next 16 bit word to I, and set PC behind it, this is a four byte instruction.
*/
void Chip8::Fun_F000()
{
	uint16_t word = Fetch(); // Get next 2 bytes to get the value of I and increment pc

	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("DW F000 %.4X\n", word); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("i := long %.4X\n", word); break;
	}

	m_indexRegister = word;

	// PC alredy incremented
}

/*
FX01 - DW #Fx01

Select bit planes to draw on when drawing with Dxy0/Dxyn.
*/
void Chip8::Fun_Fx01()
{
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("DW #F%x01\n", m_decoder.x); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("plane %d\n", m_decoder.x); break;
	}

	m_display.SetActivePlane(m_decoder.x);
}

/*
FX02 - DW #Fx02
Load 16 bytes audio pattern pointed to by I into audio pattern buffer.
*/
void Chip8::Fun_F002()
{
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("DW #F002\n"); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("audio\n"); break;
	}

	m_audio.LoadPatternBuffer(&m_memory[m_indexRegister]);
}

/*
Fx07 - LD Vx, DT
Set Vx = delay timer value.

The value of DT is placed into Vx.
*/
void Chip8::Fun_Fx07()
{
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("LD V%X, DT\n", m_decoder.x); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("v%X := delay\n", m_decoder.x); break;
	}

	m_V[m_decoder.x] = m_delayTimer;
}

/*
Fx0A - LD Vx, K
Wait for a key press, store the value of the key in Vx.

All execution stops until a key is pressed, then the value of that key is stored in Vx.
*/
void Chip8::Fun_Fx0A()
{
	//LOG("LD V%X, K (Waiting for key press...)\n", m_decoder.x);

	int key = -1;
	if (m_keyboard.WaitForInput(&key))
	{
		m_V[m_decoder.x] = key;

		switch (m_logggingMode)
		{
		case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("LD V%X, K (KeyPressed)\n", m_decoder.x); break;
		case Chip8::LOGGINGMODE::OCTO_LOG: LOG("v%X := key (KeyPressed)\n", m_decoder.x); break;
		}
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
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("LD DT, V%X\n", m_decoder.x); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("delay := v%X\n", m_decoder.x); break;
	}

	m_delayTimer = m_V[m_decoder.x];
}

/*3
Fx18 - LD ST, Vx
Set sound timer = Vx.

ST is set equal to the value of Vx.
*/
void Chip8::Fun_Fx18()
{
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("LD ST, V%X\n", m_decoder.x); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("buzzer v%X\n", m_decoder.x); break;
	}

	m_soundTimer = m_V[m_decoder.x];
}

/*
Fx1E - ADD I, Vx
Set I = I + Vx.

The values of I and Vx are added, and the results are stored in I.
*/
void Chip8::Fun_Fx1E()
{
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("ADD I, V%X\n", m_decoder.x); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("i += v%X\n", m_decoder.x); break;
	}

	m_indexRegister += m_V[m_decoder.x];
}

/*
Fx29 - LD F, Vx
LORES FONT: Set I = location of sprite for digit Vx.

The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx.
*/
void Chip8::Fun_Fx29()
{
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("LD LF, V%X\n", m_decoder.x); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("i := hex v%X\n", m_decoder.x); break;
	}

	m_indexRegister = m_V[m_decoder.x] * 5; // The font sprites have a length of 5 bytes.
}

/*
Fx30 - LD H, Vx
HIRES FONT: Set I = location of sprite for digit Vx.

The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx.
*/
void Chip8::Fun_Fx30()
{
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("LD HF, V%X\n", m_decoder.x); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("i := bighex v%X\n", m_decoder.x); break;
	}

	m_indexRegister = m_V[m_decoder.x] * 10; // The font sprites have a length of 10 bytes.
}

/*
Fx3A - DW #Fx3A
Set audio pitch for a audio pattern playback rate of 4000*2^((vX-64)/48)Hz.
*/
void Chip8::Fun_Fx3A()
{
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("DW #F%X3A\n", m_decoder.x); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("pitch := v%X\n", m_decoder.x); break;
	}

	m_audio.SetPitch(m_V[m_decoder.x]);
}

/*
Fx33 - LD B, Vx
Store BCD representation of Vx in memory locations I, I+1, and I+2.

The interpreter takes the decimal value of Vx, and places the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.
*/
void Chip8::Fun_Fx33()
{
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("LD B, V%X\n", m_decoder.x); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("bcd v%X\n", m_decoder.x); break;
	}

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
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("LD [I], V%X\n", m_decoder.x); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("save v%X\n", m_decoder.x); break;
	}

	if (m_Memory_quirk)
	{
		for (uint8_t i = 0; i <= m_decoder.x; i++)
			m_memory[m_indexRegister++] = m_V[i];
	}
	else
	{
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
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("LD V%X, [I]\n", m_decoder.x); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("load v%X\n", m_decoder.x); break;
	}

	if (m_Memory_quirk)
	{
		for (uint8_t i = 0; i <= m_decoder.x; i++)
			m_V[i] = m_memory[m_indexRegister++];
	}
	else
	{
		for (uint8_t i = 0; i <= m_decoder.x; i++)
			m_V[i] = m_memory[m_indexRegister + i];
	}
}

/*
	Fx75 - LD R, VX
	Store V0 through VX to HP-48 RPL user flags (X <= 7 in SCHIP).
*/
void Chip8::Fun_Fx75()
{
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("LD r, V%X\n", m_decoder.x); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("saveflags v%X\n", m_decoder.x); break;
	}

	std::vector<int> flags;

	switch (m_interpreter)
	{
	case Chip8::SCHIP:
		for (uint8_t i = 0; i <= 7; i++)
			flags.push_back(m_V[i]);
		break;
	case Chip8::OCTO:
	case Chip8::XOCHIP:
		for (uint8_t i = 0; i < 16; i++)
			flags.push_back(m_V[i]);
		break;
	default:
		std::exit(-1);
		break;
	}

	m_config.SetRPLUserFlags(flags);
}

/*
	Fx85 - LD VX, R
	Read V0 through VX to HP-48 RPL user flags (X <= 7 in SCHIP).
*/
void Chip8::Fun_Fx85()
{
	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("LD V%X, r\n", m_decoder.x); break;
	case Chip8::LOGGINGMODE::OCTO_LOG: LOG("loadflags v%X\n", m_decoder.x); break;
	}

	std::vector<int> flags = m_config.GetRPLUserFlags();

	switch (m_interpreter)
	{
	case Chip8::SCHIP:
		for (uint8_t i = 0; i <= 7; i++)
			m_V[i] = flags[i];
		break;
	case Chip8::OCTO:
	case Chip8::XOCHIP:
		for (uint8_t i = 0; i < 16; i++)
			m_V[i] = flags[i];
		break;
	default:
		std::exit(-1);
		break;
	}
}

/*
F0A0 - NET HOST
Host a multiplayer session. Creates a SoftAP and opens a UDP socket.
VF = 1 on success, 0 on failure.
*/
void Chip8::Fun_FxA0()
{
	if (m_interpreter != XOCHIP) {
		LOGE("{Chip8} NET HOST called in non-XOCHIP mode!");
		abort();
	}

	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("NET HOST\n"); break;
	case Chip8::LOGGINGMODE::OCTO_LOG:  LOG("net-host\n"); break;
	}

	bool ok = m_network.Host(m_config.GetNetworkSSID(), m_config.GetNetworkPassword());
	m_V[0xF] = ok ? 1 : 0;
}

/*
F0A1 - NET JOIN
Join a hosted session as a client. Blocks until connected or handshake times out.
VF = 1 on success, 0 on failure/timeout.
*/
void Chip8::Fun_FxA1()
{
	if (m_interpreter != XOCHIP) {
		LOGE("{Chip8} NET JOIN called in non-XOCHIP mode!");
		abort();
	}

	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("NET JOIN\n"); break;
	case Chip8::LOGGINGMODE::OCTO_LOG:  LOG("net-join\n"); break;
	}

	bool ok = m_network.Join(m_config.GetNetworkSSID(), m_config.GetNetworkPassword());
	m_V[0xF] = ok ? 1 : 0;
}

/*
F0A2 - NET DISCO
Disconnect from the current session and tear down WiFi.
*/
void Chip8::Fun_FxA2()
{
	if (m_interpreter != XOCHIP) {
		LOGE("{Chip8} NET DISCO called in non-XOCHIP mode!");
		abort();
	}

	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("NET DISCO\n"); break;
	case Chip8::LOGGINGMODE::OCTO_LOG:  LOG("net-disconnect\n"); break;
	}

	m_network.Disconnect();
}

/*
F0A3 - NET SEND
Send 16 bytes starting at [I] to all peers.
VF = 1 on success, 0 if disconnected or send failed.
*/
void Chip8::Fun_FxA3()
{
	if (m_interpreter != XOCHIP) {
		LOGE("{Chip8} NET SEND called in non-XOCHIP mode!");
		abort();
	}

	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("NET SEND\n"); break;
	case Chip8::LOGGINGMODE::OCTO_LOG:  LOG("net-send\n"); break;
	}

	bool ok = m_network.Send(&m_memory[m_indexRegister], NET_PAYLOAD_SIZE);
	m_V[0xF] = ok ? 1 : 0;
}

/*
F0A4 - NET RECV
Read the oldest received packet into [I..I+15] (payload) and [I+16] (sender player_id).
VF = 1 if a packet was available, 0 if the queue was empty.
*/
void Chip8::Fun_FxA4()
{
	if (m_interpreter != XOCHIP) {
		LOGE("{Chip8} NET RECV called in non-XOCHIP mode!");
		abort();
	}

	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("NET RECV\n"); break;
	case Chip8::LOGGINGMODE::OCTO_LOG:  LOG("net-recv\n"); break;
	}

	NetworkPacket pkt;
	if (m_network.Receive(pkt)) {
		for (size_t i = 0; i < NET_PAYLOAD_SIZE; i++)
			m_memory[m_indexRegister + i] = pkt.payload[i];
		m_memory[m_indexRegister + NET_PAYLOAD_SIZE] = pkt.playerID;
		m_V[0xF] = 1;
	} else {
		m_V[0xF] = 0;
	}
}

/*
FxA5 - NET STAT
Store a network status byte into Vx.
Bit 7: connected, Bit 6: role (1=HOST), Bits 3-2: peer count, Bits 1-0: player_id - 1.
*/
void Chip8::Fun_FxA5()
{
	if (m_interpreter != XOCHIP) {
		LOGE("{Chip8} NET STAT called in non-XOCHIP mode!");
		abort();
	}

	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("NET STAT V%X\n", m_decoder.x); break;
	case Chip8::LOGGINGMODE::OCTO_LOG:  LOG("net-stat v%X\n", m_decoder.x); break;
	}

	uint8_t status = 0;
	if (m_network.IsConnected())
		status |= 0x80;
	if (m_network.GetRole() == Network::NETWORK_ROLE::HOST)
		status |= 0x40;
	status |= (m_network.GetPeerCount() & 0x03) << 2;
	uint8_t pid = m_network.GetPlayerID();
	if (pid > 0)
		status |= (pid - 1) & 0x03;

	m_V[m_decoder.x] = status;
}

/*
FxA6 - NET PEERS
Store the number of connected remote peers (0-2) into Vx.
*/
void Chip8::Fun_FxA6()
{
	if (m_interpreter != XOCHIP) {
		LOGE("{Chip8} NET PEERS called in non-XOCHIP mode!");
		abort();
	}

	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("NET PEERS V%X\n", m_decoder.x); break;
	case Chip8::LOGGINGMODE::OCTO_LOG:  LOG("net-peers v%X\n", m_decoder.x); break;
	}

	m_V[m_decoder.x] = m_network.GetPeerCount();
}

/*
FxA7 - NET PID
Store this device's player ID (1-3) into Vx. Returns 0 if not connected.
*/
void Chip8::Fun_FxA7()
{
	if (m_interpreter != XOCHIP) {
		LOGE("{Chip8} NET PID called in non-XOCHIP mode!");
		abort();
	}

	switch (m_logggingMode)
	{
	case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("NET PID V%X\n", m_decoder.x); break;
	case Chip8::LOGGINGMODE::OCTO_LOG:  LOG("net-pid v%X\n", m_decoder.x); break;
	}

	m_V[m_decoder.x] = m_network.GetPlayerID();
}

/*
F0A8 - NET WAIT
Stall execution (like Fx0A) until a packet arrives in the receive queue
or the delay timer reaches 0.
VF = 1 if woken by a packet, 0 if timed out.
*/
void Chip8::Fun_FxA8()
{
	if (m_interpreter != XOCHIP) {
		LOGE("{Chip8} NET WAIT called in non-XOCHIP mode!");
		abort();
	}

	bool hasData  = m_network.HasPending();
	bool timedOut = (m_delayTimer == 0);

	if (hasData || timedOut) {
		switch (m_logggingMode)
		{
		case Chip8::LOGGINGMODE::CHIP8_LOG: LOG("NET WAIT (done)\n"); break;
		case Chip8::LOGGINGMODE::OCTO_LOG:  LOG("net-wait (done)\n"); break;
		}
		m_V[0xF] = hasData ? 1 : 0;
	} else {
		m_pc -= 2; // Retry next cycle, exactly like Fx0A
	}
}
