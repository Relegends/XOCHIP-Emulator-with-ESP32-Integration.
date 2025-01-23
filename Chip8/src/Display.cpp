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

	m_closeWindow = false;
	m_extendedMode = false;
	m_display.resize(displayHeight * displayWidth);
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
			if (m_renderer == nullptr)
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
	SDL_PollEvent(&e);
	if (e.type == SDL_QUIT)
		m_closeWindow = true;

	if (!m_closeWindow)
	{
		SDL_RenderClear(m_renderer.get());
		Render();
		SDL_RenderPresent(m_renderer.get());
	}
	else
	{
		Close();
	}
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

void Display::ScrollUp(const unsigned int lines)
{
	for (int i = 0; i < lines; i++)
	{
		for (int row = 0; row < m_displayHeight - 1; row++)
		{
			for (int col = 0; col <= m_displayWidth - 1; col++)
			{
				m_display[m_displayWidth * row + col] = m_display[m_displayWidth * (row + 1) + col];
			}
		}
		
		for (int col = 0; col < m_displayWidth; col++)
			m_display[m_displayWidth * (m_displayHeight-1) + col] = 0;
	}
}

void Display::ScrollDown(const unsigned int lines)
{
	for (int i = 0; i < lines; i++)
	{
		for (int row = m_displayHeight - 1; row > 0; row--)
		{
			for (int col = m_displayWidth - 1; col >= 0; col--)
			{
				m_display[m_displayWidth * row + col] = m_display[m_displayWidth * (row - 1) + col];
			}
		}
		std::fill(m_display.begin(), m_display.begin() + m_displayWidth, 0);
	}
}

void Display::ScrollRight(const unsigned int lines)
{
	for (int i = 0; i < lines; i++)
	{
		for (int row = m_displayHeight - 1; row > 0; row--)
		{
			for (int col = m_displayWidth - 1; col >= 0; col--)
			{
				m_display[m_displayWidth * row + col] = m_display[m_displayWidth * row + (col - 1)];
			}
		}

		for (int row = 0; row < m_displayHeight; row++)
			m_display[m_displayWidth * row] = 0;
	}
}

void Display::ScrollLeft(const unsigned int lines)
{
	for (int i = 0; i < lines; i++)
	{
		for (int col = 0; col < m_displayWidth - 1; col++)
		{
			for (int row = 0; row <= m_displayHeight - 1; row++)
			{
				m_display[m_displayWidth * row + col] = m_display[m_displayWidth * row + (col + 1)];
			}
		}

		for (int row = 0; row < m_displayHeight; row++)
			m_display[m_displayWidth * row + (m_displayWidth - 1)] = 0;
	}
}

void Display::DrawPixel(unsigned int x, unsigned int y, bool colored)
{
	SDL_Rect pixel;

	unsigned int scaleX = static_cast<unsigned int>(m_windowWidth / m_displayWidth);
	unsigned int scaleY = static_cast<unsigned int>(m_windowHeight / m_displayHeight);

	pixel.x = x * scaleX;
	pixel.y = y * scaleY;
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
	//SDL_RenderPresent(m_renderer.get());

}

void Display::Render()
{
	for (uint8_t row = 0; row < m_displayHeight; row++)
	{
		for (uint8_t col = 0; col < m_displayWidth; col++)
		{
			Display::DrawPixel(col, row, m_display[m_displayWidth * row + col]); //BE AWARE --> COL = X; ROW = Y!!!!!!!!
		}
	}
}

void Display::SetExtendedMode(const bool enable)
{
	m_extendedMode = enable;
	if (m_extendedMode)
	{
		m_displayWidth = 128;
		m_displayHeight = 64;
	}
	else
	{
		m_displayWidth = 64;
		m_displayHeight = 32;
	}

	CLS();
	m_display.resize(m_displayWidth * m_displayHeight, 0);
	//Render(); ?
}

bool Display::ChangePixelValue(unsigned int x, unsigned int y, unsigned int col, unsigned int row, const bool value, const bool clipping)
{
	unsigned int posX, posY;

	posX = x + col;
	posY = y + row;

	if (!clipping)
	{
		// Wrapping screen
		posX %= m_displayWidth;
		posY %= m_displayHeight;
	}
	else
	{
		if (x >= m_displayWidth)
			posX %= m_displayWidth;
		else if (posX >= m_displayWidth) // Clipping
			return false;

		if (y >= m_displayHeight)
			posY %= m_displayHeight;
		else if (posY >= m_displayHeight) // Clipping
			return false;
	}

	int offset = m_displayWidth * posY + posX;

	bool conflict = (m_display[offset]); // If the pixel was alredy on --> CONFLICT

	m_display[offset] = (m_display[offset] ^ value); // Paint sprite with XOR

	return conflict;
}

bool Display::IsWindowClose()
{
	return m_closeWindow;
}
