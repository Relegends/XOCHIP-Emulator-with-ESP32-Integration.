#pragma once
#include <toml.hpp>

class Configuration
{
public:
	Configuration() = default;

	void Init(std::string configPath);

	unsigned int GetDisplayWidth() const;
	unsigned int GetDisplayHeight() const;
	uint32_t GetDisplayMainColor() const;
	uint32_t GetDisplayBackgroundColor() const;

private:
	unsigned int m_displayWidth;
	unsigned int m_displayHeight;
	uint32_t m_displayMainColor;
	uint32_t m_displayBackgroundColor;
};

