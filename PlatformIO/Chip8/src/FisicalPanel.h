#pragma once
#include <LovyanGFX.hpp>
#include <Log.h>

class FisicalPanel : public lgfx::LGFX_Device
{
	public:
		FisicalPanel();

		bool Init(const unsigned int panelWidth, const unsigned int panelHeight);

		/* 
		######## Functions inherit form lgfx::LGFX_Device #########
		
		- init()
		- fillScreen()
		- pushPixels()
		
		*/


	private:
		unsigned int m_panelWidth;
		unsigned int m_panelHeight;

		lgfx::Panel_ST7789 m_panel;	// Controller for the ST7780 panel
		lgfx::Bus_SPI m_bus;		// Comunication bus
};