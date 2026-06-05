#include "Configuration.h"

Configuration::Configuration()
{
	m_filePath = "";
	m_config = toml::parse_result{};
}

bool Configuration::Init(const std::string &configPath)
{
	LOG("{Configuration} Initializing configuration from file: %s", configPath.c_str());

	m_config = toml::parse_file(configPath);
	m_filePath = configPath;
	return true;
}


unsigned int Configuration::GetLoggingMode() const
{
	unsigned int value = m_config["Logging"]["Log"].value_or(0);

	if (value < 0 || value > 2)
		value = 0;

	return value;
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

int Configuration::GetIPF() const
{
	int value = m_config["IPF"]["IPF"].value_or(10);

	if (value < 7)
		value = 7;

	return value;
}

unsigned int Configuration::GetWindowWidth() const
{
	return m_config["Display"]["windowResolution"][0].value_or(320);
}

unsigned int Configuration::GetWindowHeight() const
{
	return m_config["Display"]["windowResolution"][1].value_or(240);
}

unsigned int Configuration::GetDisplayWidth() const
{
	return m_config["Display"]["displayResolution"][0].value_or(64);
}

unsigned int Configuration::GetDisplayHeight() const
{
	return m_config["Display"]["displayResolution"][1].value_or(32);
}

unsigned int Configuration::GetPanelWidth() const
{
    return m_config["Panel"]["panelResolution"][0].value_or(320);
}

unsigned int Configuration::GetPanelHeight() const
{
    return m_config["Panel"]["panelResolution"][1].value_or(240);
}

uint16_t Configuration::GetDisplayBackgroundColor() const
{
	return m_config["Display"]["backgroundColor"].value_or<uint16_t>(0xF7DE);
}

uint16_t Configuration::GetDisplayForeground_1Color() const
{
	return m_config["Display"]["foreground1Color"].value_or<uint16_t>(0x1B05);
}

uint16_t Configuration::GetDisplayForeground_2Color() const
{
	return m_config["Display"]["foreground2Color"].value_or<uint16_t>(0xB6E9);
}

uint16_t Configuration::GetDisplayBlendedColor() const
{
	return m_config["Display"]["blendedColor"].value_or<uint16_t>(0xB6E9);
}

unsigned int Configuration::GetAudioFrequency() const
{
	return m_config["Audio"]["frequency"].value_or(440);
}

unsigned int Configuration::GetAudioSampleRate() const
{
	return m_config["Audio"]["sampleRate"].value_or(512);
}

unsigned int Configuration::GetAudioVolume() const
{
	return m_config["Audio"]["volume"].value_or(10);
}

unsigned int Configuration::GetRotation() const
{
    return m_config["Panel"]["rotation"].value_or(0);
}

bool Configuration::GetRGBOrder() const
{
    return m_config["Panel"]["rgbOrder"].value_or(0);
}

bool Configuration::GetIsFlipY() const
{
    return m_config["Panel"]["flipY"].value_or(0);
}

std::string Configuration::GetNetworkDeviceName() const
{
	auto val = m_config["Network"]["deviceName"].value_or("CHIP8-1");
	return std::string(val);
}

uint8_t Configuration::GetNetworkWifiChannel() const
{
	int ch = m_config["Network"]["wifiChannel"].value_or(6);
	if (ch < 1 || ch > 13) ch = 6;
	return static_cast<uint8_t>(ch);
}

std::string Configuration::GetNetworkSSID() const
{
	auto val = m_config["Network"]["ssid"].value_or("CHIP8NET");
	return std::string(val);
}

std::string Configuration::GetNetworkPassword() const
{
	auto val = m_config["Network"]["password"].value_or("chip8play");
	return std::string(val);
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

std::vector<int> Configuration::GetRPLUserFlags() const
{
	auto flagsArray = m_config["UserFlags"]["userFlags"].as_array();
	std::vector<int> returnVector;
	if (flagsArray)
	{
		flagsArray->for_each([&returnVector](const toml::node& flag)
			{
				if (flag.is_integer())
					returnVector.push_back(flag.as_integer()->get());
			});
	}
	return returnVector;
}

void Configuration::SetRPLUserFlags(const std::vector<int>& updatedFlags)
{
	toml::array* flagsArray = m_config["UserFlags"]["userFlags"].as_array();
	flagsArray->clear();
	for (const int& flag : updatedFlags)
		flagsArray->push_back(flag);

	UpdateConfigFile();
}

void Configuration::UpdateConfigFile()
{
	std::ofstream file(m_filePath);
	if (!file.is_open())
	{
		std::cerr << "Error opening config file!\n";
		exit(-1);
	}
	file << m_config;
	file.close();
}