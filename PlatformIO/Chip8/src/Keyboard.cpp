#include "Keyboard.h"

Keyboard::Keyboard(const std::vector<std::pair<std::string, int>>& m_keyboard)
	: m_keyboard(m_keyboard)
{
	m_inputs = {0};
	m_previousInputs = m_inputs;
	keyPressed = -1;

	m_i2cPort = I2C_NUM_0;
	m_i2cAddress = 0x20;

	i2c_config_t i2cConfig;
	memset(&i2cConfig, 0, sizeof(i2c_config_t));

	i2cConfig.mode = I2C_MODE_MASTER;
	i2cConfig.sda_io_num = GPIO_NUM_27;
	i2cConfig.scl_io_num = GPIO_NUM_22;
	i2cConfig.sda_pullup_en = GPIO_PULLUP_ENABLE;
	i2cConfig.scl_pullup_en = GPIO_PULLUP_ENABLE;
	i2cConfig.master.clk_speed = 100000;
	i2cConfig.clk_flags = 0;

	esp_err_t err = i2c_param_config(m_i2cPort, &i2cConfig);
	if (err != ESP_OK) {
		LOGE("Failed to configure I2C: %d\n", err);
		exit(-1);
	}

	err = i2c_driver_install(m_i2cPort, i2cConfig.mode, 0, 0, 0);
	if (err != ESP_OK) {
		LOGE("Failed to install I2C driver: %d\n", err);
		exit(-1);
	}

	// Config MCP23017
	if (!WriteI2CRegister(MCP23017_IODIRA, 0xFF) || !WriteI2CRegister(MCP23017_IODIRB, 0xFF)) {
		LOGE("Failed to set I/O direction for MCP23017\n");
		exit(-1);
	}
	if (!WriteI2CRegister(MCP23017_GPPUA, 0xFF) || !WriteI2CRegister(MCP23017_GPPUB, 0xFF)) {
		LOGE("Failed to enable pull-up resistors for MCP23017\n");
		exit(-1);
	}

	LOG("Keyboard initialized successfully with I2C address 0x%02X on port %d\n", m_i2cAddress, m_i2cPort);
}

void Keyboard::ProcessInput()
{
	uint8_t gpioa, gpiob;
	if (!ReadI2CRegister(MCP23017_GPIOA, &gpioa) || !ReadI2CRegister(MCP23017_GPIOB, &gpiob)) {
		LOGE("Failed to read GPIO state from MCP23017\n");
		return;
	}

	uint16_t keyboardState = (static_cast<uint16_t>(gpiob) << 8) | gpioa;
	uint16_t pressedKeys = ~keyboardState;

	for (const auto& key : m_keyboard)
		 m_inputs[key.second] = (pressedKeys >> key.second) & 1;
	
}

void Keyboard::SavePreviousInputs()
{
	m_previousInputs = m_inputs;
}

bool Keyboard::WaitForInput(int* returnValue)
{
	if (keyPressed == -1)
	{
		for (int i = 0; i < m_inputs.size(); i++)
		{
			if (m_inputs[i] != m_previousInputs[i])
			{
				m_previousInputs = m_inputs;
				keyPressed = i;
				return false;
			}
		}
	}
	else
	{
		if (m_inputs[keyPressed])
			return false;
		else
		{
			*returnValue = keyPressed;
			keyPressed = -1;
			return true;
		}
	}
	return false;
}

int Keyboard::ReadKeyboard(const unsigned int position)
{
	return m_inputs[position];
}

bool Keyboard::ReadI2CRegister(uint8_t reg, uint8_t *data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (m_i2cAddress << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, reg, true);
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (m_i2cAddress << 1) | I2C_MASTER_READ, true);
	i2c_master_read_byte(cmd, data, I2C_MASTER_NACK);
	i2c_master_stop(cmd);
	esp_err_t err = i2c_master_cmd_begin(m_i2cPort, cmd, pdMS_TO_TICKS(100));
	i2c_cmd_link_delete(cmd);
	return err == ESP_OK;
}

bool Keyboard::WriteI2CRegister(uint8_t reg, uint8_t data)
{
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (m_i2cAddress << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, reg, true);
	i2c_master_write_byte(cmd, data, true);
	i2c_master_stop(cmd);
	esp_err_t err = i2c_master_cmd_begin(m_i2cPort, cmd, pdMS_TO_TICKS(100));
	i2c_cmd_link_delete(cmd);
	return err == ESP_OK;
}
