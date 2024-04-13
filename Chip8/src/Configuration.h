#pragma once
#include <toml.hpp>
#include <unordered_map>

class Configuration
{
public:
	Configuration(std::string configPath);

	unsigned int GetWindowWidth() const;
	unsigned int GetWindowHeight() const;
	unsigned int GetDisplayWidth() const;
	unsigned int GetDisplayHeight() const;
	uint32_t GetDisplayMainColor() const;
	uint32_t GetDisplayBackgroundColor() const;

	const std::unordered_map<char, bool> GetInputMap() const;

private:
	toml::parse_result m_config;
};

