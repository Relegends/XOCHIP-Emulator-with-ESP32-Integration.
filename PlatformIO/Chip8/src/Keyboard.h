#pragma once
#include <string>
#include <vector>
#include <array>
#include <driver/i2c_master.h>
#include <freertos/FreeRTOS.h>
#include "Log.h"

// Registros del MCP23017
#define MCP23017_IODIRA   0x00	// I/O direction register for port A
#define MCP23017_IODIRB   0x01	// I/O direction register for port B
#define MCP23017_GPPUA    0x0C	// Pull-up resistor register for port A
#define MCP23017_GPPUB    0x0D	// Pull-up resistor register for port B
#define MCP23017_GPIOA    0x12	// GPIO register for port A
#define MCP23017_GPIOB    0x13	// GPIO register for port B

class Keyboard
{
public:
	Keyboard();

	bool Init(const std::vector<std::pair<std::string, int>>& keyboardConfig);

	void ProcessInput();

	void SavePreviousInputs();

	bool WaitForInput(int* returnValue);

	int ReadKeyboard(const unsigned int position);

private:

	bool ReadI2CRegister(uint8_t reg, uint8_t* data);
	bool WriteI2CRegister(uint8_t reg, uint8_t data);

private:
	std::vector<std::pair<std::string, int>> m_keyboard;
	std::array<int, 16> m_inputs;
	std::array<int, 16> m_previousInputs;
	int keyPressed;

	// I2C configuration
	i2c_master_bus_handle_t m_i2cBus;
	i2c_master_dev_handle_t m_i2cDevice;

};
