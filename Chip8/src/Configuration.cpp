#include "Configuration.h"
#include <unordered_map>
#include <iostream>

void Configuration::Init(std::string configPath)
{
	m_config = toml::parse_file(configPath);
	std::unordered_map<char, bool> a;
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

}

unsigned int Configuration::GetDisplayWidth() const
{
	return m_config["Display"]["resolution"][0].value_or(0);
}

unsigned int Configuration::GetDisplayHeight() const
{
	return m_config["Display"]["resolution"][1].value_or(0);
}

uint32_t Configuration::GetDisplayMainColor() const
{
	return m_config["Display"]["mainColor"].value_or<uint32_t>(0xFFFFFFFF);
}

uint32_t Configuration::GetDisplayBackgroundColor() const
{
	return m_config["Display"]["backgroundColor"].value_or<uint32_t>(0x00000000);
}

void Configuration::GetInputMap(std::unordered_map<char, bool>& m_keyboard) const
{
}

