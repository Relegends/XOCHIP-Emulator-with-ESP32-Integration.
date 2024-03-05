#include "Display.h"
#include <iostream>

bool Display::Init(const std::string& name, const unsigned int width, const unsigned int height)
{
	m_windowName = name;
	m_windowWidth = width;
	m_windowHeight = height;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
		return false;
	}
	else
	{
		m_window = std::unique_ptr<SDL_Window, std::function<void(SDL_Window*)>>(
			SDL_CreateWindow(m_windowName.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_windowWidth, m_windowHeight, SDL_WINDOW_SHOWN),
			SDL_DestroyWindow
		);

		if (m_window == nullptr)
		{
			std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
			return false;
		}
		else
		{
			m_renderer = std::unique_ptr<SDL_Renderer, std::function<void(SDL_Renderer*)>>(
				SDL_CreateRenderer(m_window.get(), NULL, SDL_RENDERER_ACCELERATED),
				SDL_DestroyRenderer
			);

			if (m_renderer = nullptr)
			{
				std::cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
				return false;
			}
		}

	}

	return true;
}

void Display::Update()
{
	SDL_Event e;

	while (!m_closeWindow)
	{
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
				m_closeWindow = true;
		}

		//Update the surface
		SDL_UpdateWindowSurface(m_window.get());
	}

	Close();
}

void Display::Close()
{
	m_renderer.reset();
	m_window.reset();

	SDL_Quit();
}

void Display::CLS()
{
	SDL_RenderClear(m_renderer.get());
}
