#include "Display.h"
#include <iostream>

Display::Display(const std::string& name,
	const unsigned int windowWidth, const unsigned int windowHeight,
	const unsigned int displayWidth, const unsigned int displayHeight,
	const uint32_t backgroundColor, const uint32_t foreground1Color,
	const uint32_t foreground2Color, const uint32_t blendedColor)
{
	m_windowName = name;
	m_windowWidth = windowWidth;
	m_windowHeight = windowHeight;
	m_displayWidth = displayWidth;
	m_displayHeight = displayHeight;
	m_backgroundColor = backgroundColor;
	m_foreground_1Color = foreground1Color;
	m_foreground_2Color = foreground2Color;
	m_blendedColor = blendedColor;

	m_closeWindow = false;
	m_extendedMode = false;

	m_activePlane = LSB_PLANE;

	m_display.resize(displayHeight * displayWidth);
	m_secondPlane.resize(displayHeight * displayWidth);
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
	switch (m_activePlane)
	{
	case Display::LSB_PLANE:
		std::fill(m_display.begin(), m_display.end(), 0);
		break;
	case Display::MSB_PLANE:
		std::fill(m_secondPlane.begin(), m_secondPlane.end(), 0);
		break;
	case Display::BOTH:
		std::fill(m_display.begin(), m_display.end(), 0);
		std::fill(m_secondPlane.begin(), m_secondPlane.end(), 0);
		break;
	default:
		printf("Not valid plane selected! Selected: %d\n", m_activePlane);
		exit(-1);
		break;
	}

	//SDL_RenderClear(m_renderer.get());
}

void Display::ScrollUp(const unsigned int lines)
{
	for (int i = 0; i < lines; i++)
	{
		for (int row = 0; row < m_displayHeight - 1; row++)
		{
			for (int col = 0; col <= m_displayWidth - 1; col++)
			{
				switch (m_activePlane)
				{
				case Display::LSB_PLANE:
					m_display[m_displayWidth * row + col] = m_display[m_displayWidth * (row + 1) + col];
					break;
				case Display::MSB_PLANE:
					m_secondPlane[m_displayWidth * row + col] = m_secondPlane[m_displayWidth * (row + 1) + col];
					break;
				case Display::BOTH:
					m_display[m_displayWidth * row + col] = m_display[m_displayWidth * (row + 1) + col];
					m_secondPlane[m_displayWidth * row + col] = m_secondPlane[m_displayWidth * (row + 1) + col];
					break;
				}

			}
		}
		for (int col = 0; col < m_displayWidth; col++)
		{
			switch (m_activePlane)
			{
			case Display::LSB_PLANE:
				m_display[m_displayWidth * (m_displayHeight - 1) + col] = 0;
				break;
			case Display::MSB_PLANE:
				m_secondPlane[m_displayWidth * (m_displayHeight - 1) + col] = 0;
				break;
			case Display::BOTH:
				m_display[m_displayWidth * (m_displayHeight - 1) + col] = 0;
				m_secondPlane[m_displayWidth * (m_displayHeight - 1) + col] = 0;
				break;
			}
		}


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
				switch (m_activePlane)
				{
				case Display::LSB_PLANE:
					m_display[m_displayWidth * row + col] = m_display[m_displayWidth * (row - 1) + col];
					break;
				case Display::MSB_PLANE:
					m_secondPlane[m_displayWidth * row + col] = m_secondPlane[m_displayWidth * (row - 1) + col];
					break;
				case Display::BOTH:
					m_display[m_displayWidth * row + col] = m_display[m_displayWidth * (row - 1) + col];
					m_secondPlane[m_displayWidth * row + col] = m_secondPlane[m_displayWidth * (row - 1) + col];
					break;
				}
			}
		}
		switch (m_activePlane)
		{
		case Display::LSB_PLANE:
			std::fill(m_display.begin(), m_display.begin() + m_displayWidth, 0);
			break;
		case Display::MSB_PLANE:
			std::fill(m_secondPlane.begin(), m_secondPlane.begin() + m_displayWidth, 0);
			break;
		case Display::BOTH:
			std::fill(m_display.begin(), m_display.begin() + m_displayWidth, 0);
			std::fill(m_secondPlane.begin(), m_secondPlane.begin() + m_displayWidth, 0);
			break;
		}

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
				switch (m_activePlane)
				{
				case Display::LSB_PLANE:
					m_display[m_displayWidth * row + col] = m_display[m_displayWidth * row + (col - 1)];
					break;
				case Display::MSB_PLANE:
					m_secondPlane[m_displayWidth * row + col] = m_secondPlane[m_displayWidth * row + (col - 1)];
					break;
				case Display::BOTH:
					m_display[m_displayWidth * row + col] = m_display[m_displayWidth * row + (col - 1)];
					m_secondPlane[m_displayWidth * row + col] = m_secondPlane[m_displayWidth * row + (col - 1)];
					break;
				}

			}
		}
		for (int row = 0; row < m_displayHeight; row++)
		{
			switch (m_activePlane)
			{
			case Display::LSB_PLANE:
				m_display[m_displayWidth * row] = 0;
				break;
			case Display::MSB_PLANE:
				m_secondPlane[m_displayWidth * row] = 0;
				break;
			case Display::BOTH:
				m_display[m_displayWidth * row] = 0;
				m_secondPlane[m_displayWidth * row] = 0;
				break;
			}
		}
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
				switch (m_activePlane)
				{
				case Display::LSB_PLANE:
					m_display[m_displayWidth * row + col] = m_display[m_displayWidth * row + (col + 1)];
					break;
				case Display::MSB_PLANE:
					m_secondPlane[m_displayWidth * row + col] = m_secondPlane[m_displayWidth * row + (col + 1)];
					break;
				case Display::BOTH:
					m_display[m_displayWidth * row + col] = m_display[m_displayWidth * row + (col + 1)];
					m_secondPlane[m_displayWidth * row + col] = m_secondPlane[m_displayWidth * row + (col + 1)];
					break;
				}
			}
		}

		for (int row = 0; row < m_displayHeight; row++)
		{
			switch (m_activePlane)
			{
			case Display::LSB_PLANE:
				m_display[m_displayWidth * row + (m_displayWidth - 1)] = 0;
				break;
			case Display::MSB_PLANE:
				m_secondPlane[m_displayWidth * row + (m_displayWidth - 1)] = 0;
				break;
			case Display::BOTH:
				m_display[m_displayWidth * row + (m_displayWidth - 1)] = 0;
				m_secondPlane[m_displayWidth * row + (m_displayWidth - 1)] = 0;
				break;
			}
		}
	}
}

void Display::DrawPixel(unsigned int x, unsigned int y, bool planeLSB, bool planeMSB)
{
	SDL_Rect pixel;

	unsigned int scaleX = static_cast<unsigned int>(m_windowWidth / m_displayWidth);
	unsigned int scaleY = static_cast<unsigned int>(m_windowHeight / m_displayHeight);

	pixel.x = x * scaleX;
	pixel.y = y * scaleY;
	pixel.w = scaleX;
	pixel.h = scaleY;

	uint8_t pixelColor = (static_cast<uint8_t>(planeMSB) << 1) | static_cast<uint8_t>(planeLSB);
	uint32_t selectedColor = 0;

	switch (pixelColor)
	{
	case BACKGROUND:
		selectedColor = m_backgroundColor;
		break;
	case FOREGROUND1:
		selectedColor = m_foreground_1Color;
		break;
	case FOREGROUND2:
		selectedColor = m_foreground_2Color;
		break;
	case BLENDED:
		selectedColor = m_blendedColor;
		break;
	default:
		printf("ERROR: PixelColor not valid! PixelColor selected: %d\n", pixelColor);
		exit(-1);
		break;
	}

	SDL_SetRenderDrawColor(m_renderer.get(),
		(selectedColor >> 16) & 0xFF, (selectedColor >> 8) & 0xFF, selectedColor & 0xFF, 0xFF);

	SDL_RenderFillRect(m_renderer.get(), &pixel);
	//SDL_RenderPresent(m_renderer.get());

}

void Display::Render()
{
	for (uint8_t row = 0; row < m_displayHeight; row++)
	{
		for (uint8_t col = 0; col < m_displayWidth; col++)
		{
			Display::DrawPixel(col, row, m_display[m_displayWidth * row + col], m_secondPlane[m_displayWidth * row + col]); //BE AWARE --> COL = X; ROW = Y!!!!!!!!
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
	m_secondPlane.resize(m_displayWidth * m_displayHeight, 0);
	//Render(); ?
}

const bool Display::GetExtendedMode() const
{
	return m_extendedMode;
}

void Display::SetActivePlane(const unsigned int activePlane)
{
	m_activePlane = static_cast<ACTIVEPLANE>(activePlane);
}

const Display::ACTIVEPLANE Display::GetActivePlane() const
{
	return m_activePlane;
}

bool Display::ChangePixelValue(unsigned int x, unsigned int y, unsigned int col, unsigned int row, const bool value, const bool clipping, const ACTIVEPLANE drawingPlaneOnBothMode)
{
	unsigned int posX, posY;
	bool conflict = false;

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

	switch (m_activePlane)
	{
	case Display::LSB_PLANE:

		conflict = (m_display[offset]); // If the pixel was alredy on --> CONFLICT
		m_display[offset] = (m_display[offset] ^ value); // Paint sprite with XOR

		break;
	case Display::MSB_PLANE:

		conflict = (m_secondPlane[offset]); // If the pixel was alredy on --> CONFLICT
		m_secondPlane[offset] = (m_secondPlane[offset] ^ value); // Paint sprite with XOR

		break;
	case Display::BOTH:

		switch (drawingPlaneOnBothMode)
		{
		case Display::LSB_PLANE:
			conflict = (m_display[offset]); // If the pixel was alredy on --> CONFLICT
			m_display[offset] = (m_display[offset] ^ value); // Paint sprite with XOR
			break;
		case Display::MSB_PLANE:
			conflict = (m_secondPlane[offset]); // If the pixel was alredy on --> CONFLICT
			m_secondPlane[offset] = (m_secondPlane[offset] ^ value); // Paint sprite with XOR
			break;
		default:
			printf("Not valid plane selected! Selected: %d\n", m_activePlane);
			exit(-1);
			break;
		}
	}

	return conflict;

}

bool Display::IsWindowClose() const
{
	return m_closeWindow;
}
