#include "Configuration.h"

void Configuration::Init(std::string configPath)
{
	auto config = toml::parse_file(configPath);

	// Display config
	m_displayWidth = config["Display"]["resolution"][0].value_or(0);
	m_displayHeight = config["Display"]["resolution"][1].value_or(0);
	m_displayMainColor = config["Display"]["mainColor"].value_or(0);
	m_displayBackgroundColor = config["Display"]["backgroundColor"].value_or(0);
}

unsigned int Configuration::GetDisplayWidth() const
{
	return m_displayWidth;
}

unsigned int Configuration::GetDisplayHeight() const
{
	return m_displayHeight;
}

uint32_t Configuration::GetDisplayMainColor() const
{
	return m_displayMainColor;
}

uint32_t Configuration::GetDisplayBackgroundColor() const
{
	return m_displayBackgroundColor;
}
