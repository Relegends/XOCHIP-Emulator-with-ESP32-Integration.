#pragma once
#include <toml.hpp>
#include <vector>
#include <iostream>
#include <Log.h>

class Configuration
{
public:
	Configuration();

	bool Init(const std::string& configPath);

	unsigned int GetLoggingMode() const;
	unsigned int GetInterpretedMode() const;
	int GetVFresetQuirk() const;
	int GetMemoryQuirk() const;
	int GetDisplayWaitQuirk() const;
	int GetClippingQuirk() const;
	int GetShiftingQuirk() const;
	int GetJumpingQuirk() const;
	int GetIPF() const;
	unsigned int GetWindowWidth() const;
	unsigned int GetWindowHeight() const;
	unsigned int GetDisplayWidth() const;
	unsigned int GetDisplayHeight() const;
	uint32_t GetDisplayBackgroundColor() const;
	uint32_t GetDisplayForeground_1Color() const;
	uint32_t GetDisplayForeground_2Color() const;
	uint32_t GetDisplayBlendedColor() const;
	unsigned int GetAudioFrequency() const;
	unsigned int GetAudioSampleRate() const;
	unsigned int GetAudioVolume() const;

	std::vector<std::pair<std::string, int>> GetInputMap() const;

	std::vector<int> GetRPLUserFlags() const;
	void SetRPLUserFlags(const std::vector<int>& updatedFlags);

private:
	void UpdateConfigFile();

private:
	toml::parse_result m_config;
	std::string m_filePath;
};

