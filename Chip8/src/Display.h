#pragma once
#include <string>
#include <memory>
#include <array>
#include <functional>
#include <SDL.h>

class Display
{
public:
	Display(const std::string& name,
		const unsigned int windowWidth, const unsigned int windowHeight,
		const unsigned int displayWidth, const unsigned int displayHeight,
		const uint32_t mainColor, const uint32_t backgroundColor);

	bool Init();
	void Update();
	void Close();

	void CLS();	// Clear Screen

	void Render();

	bool ChangePixelValue(unsigned int x, unsigned int y, const bool value); // Returns wheter there was a collision or not

	bool IsWindowClose();

private:

	void DrawPixel(unsigned int x, unsigned int y, bool colored);

private:
	unsigned int m_windowHeight;
	unsigned int m_windowWidth;
	uint32_t m_mainColor;
	uint32_t m_backgroundColor;

	bool m_closeWindow;

	unsigned int m_displayWidth;
	unsigned int m_displayHeight;

	std::string m_windowName;

	std::vector<bool> m_display;

	std::unique_ptr < SDL_Window, std::function<void(SDL_Window*)>> m_window;
	std::unique_ptr<SDL_Renderer, std::function<void(SDL_Renderer*)>> m_renderer;
};

