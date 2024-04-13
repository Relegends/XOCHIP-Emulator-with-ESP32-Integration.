#include "Configuration.h"

Configuration::Configuration(std::string configPath)
{
	m_config = toml::parse_file(configPath);
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

uint32_t Configuration::GetDisplayMainColor() const
{
	return m_config["Display"]["mainColor"].value_or<uint32_t>(0xFFFFFFFF);
}

uint32_t Configuration::GetDisplayBackgroundColor() const
{
	return m_config["Display"]["backgroundColor"].value_or<uint32_t>(0x00000000);
}

const std::unordered_map<char, bool> Configuration::GetInputMap() const
{
	auto arr = m_config["Keyboard"].as_table();
	std::unordered_map<char, bool> map;
	arr->for_each([&map](auto&& el)
		{
			if constexpr (toml::is_string<decltype(el)>)
			{
				std::string elem = toml::value<std::string>(el).get();
				map[elem[0]] = 0;
			}
		});
	return map;
}

