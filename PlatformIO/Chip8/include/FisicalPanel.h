#pragma once
#include <LovyanGFX.hpp>

class FisicalPanel : public lgfx::LGFX_Device
{
	public:
		FisicalPanel(const unsigned int panelWidth, const unsigned int panelHeight);

		/* 
		######## Functions inherit form lgfx::LGFX_Device #########
		
		- init()
		- fillScreen()
		- pushPixels()
		
		*/


	private:
		lgfx::Panel_ST7789 m_panel;	// Controller for the ST7780 panel
		lgfx::Bus_SPI m_bus;		// Comunication bus
};