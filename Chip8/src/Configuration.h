#pragma once
#include <toml.hpp>
#include <vector>

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

	std::vector<std::pair<std::string, int>> GetInputMap() const;

private:
	toml::parse_result m_config;
};

