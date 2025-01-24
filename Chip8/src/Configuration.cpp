#include "Configuration.h"

Configuration::Configuration(std::string configPath)
{
	m_config = toml::parse_file(configPath);
}

unsigned int Configuration::GetInterpretedMode() const
{
	unsigned int value = m_config["InterpreterMode"]["interpreter"].value_or(0);

	if (value < 0 || value > 3)
		value = 2;

	return value;
}

int Configuration::GetVFresetQuirk() const
{
	int value = m_config["Quirks"]["VFreset"].value_or(-1);

	if (value < -1 || value > 1)
		value = -1;

	return value;
}

int Configuration::GetMemoryQuirk() const
{
	int value = m_config["Quirks"]["Memory"].value_or(-1);

	if (value < -1 || value > 1)
		value = -1;

	return value;
}

int Configuration::GetDisplayWaitQuirk() const
{
	int value = m_config["Quirks"]["DisplayWait"].value_or(-1);

	if (value < -1 || value > 1)
		value = -1;

	return value;
}

int Configuration::GetClippingQuirk() const
{
	int value = m_config["Quirks"]["Clipping"].value_or(-1);

	if (value < -1 || value > 1)
		value = -1;

	return value;
}

int Configuration::GetShiftingQuirk() const
{
	int value = m_config["Quirks"]["Shifting"].value_or(-1);

	if (value < -1 || value > 1)
		value = -1;

	return value;
}

int Configuration::GetJumpingQuirk() const
{
	int value = m_config["Quirks"]["Jumping"].value_or(-1);

	if (value < -1 || value > 1)
		value = -1;

	return value;
}

int Configuration::GetIPS() const
{
	int value = m_config["IPS"]["IPS"].value_or(10);

	if (value < 7)
		value = 7;

	return value;
}

unsigned int Configuration::GetWindowWidth() const
{
	return m_config["Display"]["windowResolution"][0].value_or(1920);
}

unsigned int Configuration::GetWindowHeight() const
{
	return m_config["Display"]["windowResolution"][1].value_or(1080);
}

unsigned int Configuration::GetDisplayWidth() const
{
	return m_config["Display"]["displayResolution"][0].value_or(64);
}

unsigned int Configuration::GetDisplayHeight() const
{
	return m_config["Display"]["displayResolution"][1].value_or(32);
}

uint32_t Configuration::GetDisplayBackgroundColor() const
{
	uint32_t color = m_config["Display"]["backgroundColor"].value_or<uint32_t>(0x000000);
	return color >> 8;
}

uint32_t Configuration::GetDisplayForeground_1Color() const
{
	uint32_t color = m_config["Display"]["foreground1Color"].value_or<uint32_t>(0xFFFFFFFF);
	return color >> 8;
}

uint32_t Configuration::GetDisplayForeground_2Color() const
{
	uint32_t color = m_config["Display"]["foreground2Color"].value_or<uint32_t>(0x222222FF);
	return color >> 8;
}

uint32_t Configuration::GetDisplayBlendedColor() const
{
	uint32_t color = m_config["Display"]["blendedColor"].value_or<uint32_t>(0x333333FF);
	return color >> 8;
}

std::vector<std::pair<std::string, int>> Configuration::GetInputMap() const
{
	auto tbl = m_config["Keyboard"].as_table();
	std::vector<std::pair<std::string, int>> vec;
	tbl->for_each([&vec](auto&& key)
		{
			std::pair<std::string, int> pairValue{};

			toml::array input(key);
			for (auto&& elem : input)
			{
				elem.visit([&pairValue](auto&& el) noexcept
					{
						if constexpr (toml::is_string<decltype(el)>)
						{
							std::string keyName = toml::value<std::string>(el).get();
							pairValue.first = keyName;
						}
						else if constexpr (toml::is_number<decltype(el)>)
							pairValue.second = *el;
					});
			}

			vec.push_back(pairValue);
		});
	return vec;
}

