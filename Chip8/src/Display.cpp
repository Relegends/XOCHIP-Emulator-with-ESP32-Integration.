#include "Display.h"
#include <iostream>

Display::Display(const std::string& name,
	const unsigned int windowWidth, const unsigned int windowHeight,
	const unsigned int displayWidth, const unsigned int displayHeight,
	const uint32_t mainColor, const uint32_t backgroundColor)
{
	m_windowName = name;
	m_windowWidth = windowWidth;
	m_windowHeight = windowHeight;
	m_displayWidth = displayWidth;
	m_displayHeight = displayHeight;
	m_mainColor = mainColor;
	m_backgroundColor = backgroundColor;
}

bool Display::Init()
{
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

	Display::CLS();

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
	std::fill(m_display.begin(), m_display.end(), 0);
	SDL_RenderClear(m_renderer.get());
}

void Display::DrawPixel(unsigned int x, unsigned int y, bool colored)
{
	SDL_Rect pixel;

	unsigned int scaleX = static_cast<unsigned int>(m_windowWidth / 64);
	unsigned int scaleY = static_cast<unsigned int>(m_windowHeight / 32);

	pixel.x = x;
	pixel.y = y;
	pixel.w = scaleX;
	pixel.h = scaleY;

	if (colored)
		SDL_SetRenderDrawColor(m_renderer.get(),
			(m_mainColor >> 24) & 0xFF, (m_mainColor >> 16) & 0xFF, (m_mainColor >> 8) & 0xFF,
			m_mainColor & 0xFF);
	else
		SDL_SetRenderDrawColor(m_renderer.get(), 
			(m_backgroundColor >> 24) & 0xFF, (m_backgroundColor >> 16) & 0xFF, (m_backgroundColor >> 8) & 0xFF,
			m_backgroundColor & 0xFF);

	SDL_RenderFillRect(m_renderer.get(), &pixel);
}

void Display::Render()
{
	for (uint8_t row = 0; row < m_displayHeight; row++)
	{
		for (uint8_t col = 0; col < m_displayWidth; col++)
		{
			Display::DrawPixel(row, col, m_display[m_displayWidth * row + col]);
		}
	}
}

bool Display::ChangePixelValue(unsigned int x, unsigned int y, const bool value)
{
	// Wrapping screen
	x %= m_displayWidth;
	y %= m_displayHeight;

	int offset = m_displayWidth * y + x;

	bool conflict = (m_display[offset]); // If the pixel was alredy on --> CONFLICT

	m_display[offset] = (m_display[offset] ^ value); // Paint sprite with XOR

	return conflict;
}
