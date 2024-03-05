#pragma once
#include <string>
#include <memory>
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

private:
	unsigned int m_windowHeight;
	unsigned int m_windowWidth;
	bool m_closeWindow;

	std::string m_windowName;
	std::unique_ptr < SDL_Window, std::function<void(SDL_Window*)>> m_window;
	std::unique_ptr<SDL_Renderer, std::function<void(SDL_Renderer*)>> m_renderer;
};

