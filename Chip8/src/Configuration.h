#pragma once
#include <toml.hpp>
#include <unordered_map>

class Configuration
{
public:
	Configuration() = default;

	void Init(std::string configPath);

	unsigned int GetDisplayWidth() const;
	unsigned int GetDisplayHeight() const;
	uint32_t GetDisplayMainColor() const;
	uint32_t GetDisplayBackgroundColor() const;

	void GetInputMap(std::unordered_map<char, bool>& m_keyboard) const;

private:
	toml::parse_result m_config;
};

