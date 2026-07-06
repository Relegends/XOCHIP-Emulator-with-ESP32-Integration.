#pragma once
#include <LovyanGFX.hpp>
#include <Log.h>

class FisicalPanel : public lgfx::LGFX_Device
{
	public:
		FisicalPanel();

		bool Init(const unsigned int panelWidth, const unsigned int panelHeight, const unsigned int panelRotation, const bool rgbOrder, const bool flipY);

		
        /* 
		######## Functions inherit form lgfx::LGFX_Device #########
		
		- init()
		- fillScreen()
		- pushPixels()
		
		*/
	
	private:
		void InitBus();
        void InitBackLight();
        void InitPanel();

	private:
		unsigned int m_panelWidth;
		unsigned int m_panelHeight;
		unsigned int m_panelRotation;
		bool m_rgbOrder;
		bool m_flipY;

		lgfx::Panel_ILI9341 m_panel;	// Controller for the ILI9341 panel
		lgfx::Light_PWM m_light;		// Backlight of the panel
		lgfx::Bus_SPI m_bus;			// Comunication bus
};