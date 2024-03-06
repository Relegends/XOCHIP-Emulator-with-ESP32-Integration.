#pragma once
#include <string>
#include <memory>
#include <array>
#include <functional>
#include <SDL.h>

class Display
{
public:
	Display() = default;
	~Display() = default;

	bool Init(const std::string& name, const unsigned int width, const unsigned int height);
	void Update();
	void Close();

	void CLS();	// Clear Screen

	void Render();

	bool ChangePixelValue(unsigned int x, unsigned int y, const bool value); // Returns wheter there was a collision or not

private:

	void DrawPixel(unsigned int x, unsigned int y, bool colored);

private:
	unsigned int m_windowHeight;
	unsigned int m_windowWidth;
	bool m_closeWindow;

	unsigned int m_displayWidth;
	unsigned int m_displayHeight;

	std::string m_windowName;

	std::vector<bool> m_display;

	std::unique_ptr < SDL_Window, std::function<void(SDL_Window*)>> m_window;
	std::unique_ptr<SDL_Renderer, std::function<void(SDL_Renderer*)>> m_renderer;
};

