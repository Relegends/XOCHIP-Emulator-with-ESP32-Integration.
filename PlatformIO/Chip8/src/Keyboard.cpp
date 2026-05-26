#include "Keyboard.h"

Keyboard::Keyboard()
{
	m_keyboard = {};
	m_inputs = {0};
	m_previousInputs = m_inputs;
	keyPressed = -1;

	
}

bool Keyboard::Init(const std::vector<std::pair<std::string, int>> &keyboardConfig)
{
	LOG("{Keyboard} Initializing keyboard with I2C...");

	m_keyboard = keyboardConfig;
	m_inputs = {0};
	m_previousInputs = m_inputs;
	keyPressed = -1;

	i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = GPIO_NUM_27,
        .scl_io_num = GPIO_NUM_26,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags = {
            .enable_internal_pullup = true,
        },
    };

	esp_err_t err = i2c_new_master_bus(&bus_cfg, &m_i2cBus);
    if (err != ESP_OK) {
        LOGE("{Keyboard} Failed to create I2C bus: %s", esp_err_to_name(err));
        return false;
    }

	i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x24,   // MCP23017 address
        .scl_speed_hz = 10000,
    };

    err = i2c_master_bus_add_device(m_i2cBus, &dev_cfg, &m_i2cDevice);
	if (err != ESP_OK) {
		LOGE("{Keyboard} Failed to install I2C driver: %d", err);
		return false;
	}

	// Config MCP23017
	if (!WriteI2CRegister(MCP23017_IODIRA, 0xFF) || !WriteI2CRegister(MCP23017_IODIRB, 0xFF)) {
		LOGE("{Keyboard}Failed to set I/O direction for MCP23017");
		return false;
	}
	if (!WriteI2CRegister(MCP23017_GPPUA, 0xFF) || !WriteI2CRegister(MCP23017_GPPUB, 0xFF)) {
		LOGE("{Keyboard} Failed to enable pull-up resistors for MCP23017");
		return false;
	}

	LOG("{Keyboard} Keyboard initialized successfully with I2C");
    return true;
}

void Keyboard::ProcessInput()
{
	uint8_t gpioa, gpiob;
	if (!ReadI2CRegister(MCP23017_GPIOA, &gpioa) || !ReadI2CRegister(MCP23017_GPIOB, &gpiob)) {
		LOGE("{Keyboard} Failed to read GPIO state from MCP23017");
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
    uint8_t write_buf[1] = { reg };
    return i2c_master_transmit_receive(m_i2cDevice, write_buf, 1, data, 1, pdMS_TO_TICKS(100)) == ESP_OK;
}

bool Keyboard::WriteI2CRegister(uint8_t reg, uint8_t data)
{
	uint8_t buf[2] = { reg, data };
    return i2c_master_transmit(m_i2cDevice, buf, 2, pdMS_TO_TICKS(100)) == ESP_OK;
}
