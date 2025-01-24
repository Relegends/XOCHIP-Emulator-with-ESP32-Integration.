#pragma once
#include <toml.hpp>
#include <vector>

class Configuration
{
public:
	Configuration(std::string configPath);

	unsigned int GetInterpretedMode() const;
	int GetVFresetQuirk() const;
	int GetMemoryQuirk() const;
	int GetDisplayWaitQuirk() const;
	int GetClippingQuirk() const;
	int GetShiftingQuirk() const;
	int GetJumpingQuirk() const;
	int GetIPS() const;
	unsigned int GetWindowWidth() const;
	unsigned int GetWindowHeight() const;
	unsigned int GetDisplayWidth() const;
	unsigned int GetDisplayHeight() const;
	uint32_t GetDisplayBackgroundColor() const;
	uint32_t GetDisplayForeground_1Color() const;
	uint32_t GetDisplayForeground_2Color() const;
	uint32_t GetDisplayBlendedColor() const;

	std::vector<std::pair<std::string, int>> GetInputMap() const;

private:
	toml::parse_result m_config;
};

