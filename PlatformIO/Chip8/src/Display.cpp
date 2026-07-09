#include "Display.h"
#include <iostream>

Display::Display() :
	m_panel(),
	m_sprite(&m_panel)
{
	m_windowWidth = 0;
	m_windowHeight = 0;
	m_displayWidth = 0;
	m_displayHeight = 0;
	m_backgroundColor = 0;
	m_foreground_1Color = 0;
	m_foreground_2Color = 0;
	m_blendedColor = 0;

	m_closeWindow = false;
	m_needsUpdate = false;
	m_extendedMode = false;
	m_isFlipY = false;

	m_scaleX = 1.0f;
	m_scaleY = 1.0f;

	m_activePlane = LSB_PLANE;
}

bool Display::Init(
	const unsigned int windowWidth, const unsigned int windowHeight,
	const unsigned int displayWidth, const unsigned int displayHeight,
	const uint16_t backgroundColor, const uint16_t foreground1Color,
	const uint16_t foreground2Color, const uint16_t blendedColor,
	const unsigned int panelWidth, const unsigned int panelHeight,
	const unsigned int panelRotation, const bool rgbOrder,
	const bool isFlipY)
{
	LOG("{Display} Initializing display...");

	m_windowWidth = windowWidth;
	m_windowHeight = windowHeight;
	m_displayWidth = displayWidth;
	m_displayHeight = displayHeight;
	m_isFlipY = isFlipY;

    m_backgroundColor = backgroundColor;
    m_foreground_1Color = foreground1Color;
    m_foreground_2Color = foreground2Color;
    m_blendedColor = blendedColor;

	// Pre-allocate at SCHIP max so SetExtendedMode doesn't resize and fragment the heap
	// before the 16384-byte sprite reallocation (resize × 2 would eat the 16384 free block).
	m_display.resize(128 * 64, false);
	m_secondPlane.resize(128 * 64, false);

	UpdateScale();

	if (!m_panel.Init(panelWidth, panelHeight, panelRotation, rgbOrder, isFlipY))
	{
		LOG("{Display} ERROR! Failed to initialize physical panel!");
		return false;
	}

	m_sprite.setColorDepth(16);
	m_sprite.createSprite(displayWidth, displayHeight);

	Display::CLS();

	m_panel.startWrite(); // Hold the SPI bus for the lifetime of the session
	m_panel.fillScreen(m_backgroundColor); // Clear full panel GRAM so borders are clean

	LOG("{Display} Panel Width: %d, Panel Height: %d, Display Width: %d, Display Height: %d", m_windowWidth, m_windowHeight, m_displayWidth, m_displayHeight);
	LOG("{Display} BackgroundColor: %04X, Foreground1Color: %04X, Foreground2Color: %04X, BlendedColor: %04X", m_backgroundColor, m_foreground_1Color, m_foreground_2Color, m_blendedColor);
	LOG("{Display} Initialized successfully!");

	return true;
}


void Display::Update()
{
	if (!m_closeWindow)
	{
		if (m_needsUpdate)
		{
			Render();
			m_needsUpdate = false;
		}
	}
	else
	{
		Close();
	}
}

void Display::Close()
{
	m_panel.endWrite();
	m_panel.fillScreen(0);
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
		LOGE("{Display} Not valid plane selected! Selected: %d", m_activePlane);
		abort();
		break;
	}

	m_sprite.fillSprite(m_backgroundColor);
	m_needsUpdate = true;
}

void Display::ScrollUp(const unsigned int lines)
{
	m_needsUpdate = true;
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
	m_needsUpdate = true;
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
	m_needsUpdate = true;
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
	m_needsUpdate = true;
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

// DEPRECATED VERSION!

// void Display::DrawPixel(unsigned int x, unsigned int y, bool planeLSB, bool planeMSB)
// {
// 	SDL_Rect pixel;

// 	unsigned int scaleX = static_cast<unsigned int>(m_windowWidth / m_displayWidth);
// 	unsigned int scaleY = static_cast<unsigned int>(m_windowHeight / m_displayHeight);

// 	pixel.x = x * scaleX;
// 	pixel.y = y * scaleY;
// 	pixel.w = scaleX;
// 	pixel.h = scaleY;

// 	uint8_t pixelColor = (static_cast<uint8_t>(planeMSB) << 1) | static_cast<uint8_t>(planeLSB);
// 	uint32_t selectedColor = 0;

// 	switch (pixelColor)
// 	{
// 	case BACKGROUND:
// 		selectedColor = m_backgroundColor;
// 		break;
// 	case FOREGROUND1:
// 		selectedColor = m_foreground_1Color;
// 		break;
// 	case FOREGROUND2:
// 		selectedColor = m_foreground_2Color;
// 		break;
// 	case BLENDED:
// 		selectedColor = m_blendedColor;
// 		break;
// 	default:
// 		printf("ERROR: PixelColor not valid! PixelColor selected: %d\n", pixelColor);
// 		exit(-1);
// 		break;
// 	}

// 	SDL_SetRenderDrawColor(m_renderer.get(),
// 		(selectedColor >> 16) & 0xFF, (selectedColor >> 8) & 0xFF, selectedColor & 0xFF, 0xFF);

// 	SDL_RenderFillRect(m_renderer.get(), &pixel);
// }

void Display::DrawPixelToCombineBuffer(unsigned int x, unsigned int y, bool planeLSB, bool planeMSB)
{
	uint8_t pixelColor = (static_cast<uint8_t>(planeMSB) << 1) | static_cast<uint8_t>(planeLSB);
	uint16_t selectedColor = 0;

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

	if (m_sprite.getBuffer() == nullptr) return;

	unsigned int offset = m_displayWidth * y + x;
	((uint16_t*)m_sprite.getBuffer())[offset] = selectedColor;
}

/*
void Display::UpdateTexture()
{
    const unsigned int scale = std::min(m_windowWidth  / m_displayWidth,
                                        m_windowHeight / m_displayHeight);
    const unsigned int offsetY = (m_windowHeight - m_displayHeight * scale) / 2;
    const unsigned int offsetX = (m_windowWidth  - m_displayWidth  * scale) / 2;

    std::vector<uint16_t> rowBuffer(m_windowWidth, m_backgroundColor);

    for (unsigned int py = 0; py < offsetY; py++)
    {
        std::fill(rowBuffer.begin(), rowBuffer.end(), m_backgroundColor);
        m_panel.pushImage(0, py, m_windowWidth, 1, rowBuffer.data(), m_panel.getColorDepth());
    }

    for (unsigned int row = 0; row < m_displayHeight; row++)
    {
        std::fill(rowBuffer.begin(), rowBuffer.end(), m_backgroundColor);
        for (unsigned int col = 0; col < m_displayWidth; col++)
        {
            uint16_t color = m_combineBuffer[m_displayWidth * row + col];
            for (unsigned int sx = 0; sx < scale; sx++)
                rowBuffer[offsetX + col * scale + sx] = color;
        }

        for (unsigned int sy = 0; sy < scale; sy++)
        {
            unsigned int py = offsetY + row * scale + sy;
            m_panel.pushImage(0, py, m_windowWidth, 1, rowBuffer.data(), m_panel.getColorDepth());
        }
    }

    for (unsigned int py = offsetY + m_displayHeight * scale; py < m_windowHeight; py++)
    {
        std::fill(rowBuffer.begin(), rowBuffer.end(), m_backgroundColor);
        m_panel.pushImage(0, py, m_windowWidth, 1, rowBuffer.data(), m_panel.getColorDepth());
    }
}
*/

void Display::UpdateScale()
{
	m_scaleX = (float)m_windowWidth  / m_displayWidth;
	float sy  = (float)m_windowHeight / m_displayHeight;
	m_scaleY  = m_isFlipY ? -sy : sy;
}

void Display::UpdateTexture()
{
	m_panel.pushImageRotateZoom(
		m_windowWidth  / 2.0f,
		m_windowHeight / 2.0f,
		m_displayWidth  / 2.0f,
		m_displayHeight / 2.0f,
		0.0f,
		m_scaleX, m_scaleY,
		m_displayWidth, m_displayHeight,
		(uint16_t*)m_sprite.getBuffer()
	);
}

void Display::Render()
{
	for (uint8_t row = 0; row < m_displayHeight; row++)
	{
		for (uint8_t col = 0; col < m_displayWidth; col++)
		{
			//Display::DrawPixel(col, row, m_display[m_displayWidth * row + col], m_secondPlane[m_displayWidth * row + col]); //BE AWARE --> COL = X; ROW = Y!!!!!!!!
			Display::DrawPixelToCombineBuffer(col, row, m_display[m_displayWidth * row + col], m_secondPlane[m_displayWidth * row + col]);
		}
	}

	Display::UpdateTexture();
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

	// Sprite is pre-allocated at 128×64 by AllocSprite() — no reallocation needed.
	// Vectors are pre-allocated at 128×64 in Init() — no resize needed.
	UpdateScale();
	m_panel.fillScreen(m_backgroundColor);
	CLS();
}

void Display::FreeSprite()
{
	m_sprite.deleteSprite();
}

void Display::AllocSprite()
{
	m_sprite.setColorDepth(16);
	if (!m_sprite.createSprite(128, 64))
	{
		LOGE("{Display} FATAL: AllocSprite failed (128x64, 16384 bytes)!");
		return;
	}
	CLS();
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

		conflict = (m_display[offset] && value); // If the pixel was alredy on --> CONFLICT
		m_display[offset] = (m_display[offset] ^ value); // Paint sprite with XOR

		break;
	case Display::MSB_PLANE:

		conflict = (m_secondPlane[offset] && value); // If the pixel was alredy on --> CONFLICT
		m_secondPlane[offset] = (m_secondPlane[offset] ^ value); // Paint sprite with XOR

		break;
	case Display::BOTH:

		switch (drawingPlaneOnBothMode)
		{
		case Display::LSB_PLANE:
			conflict = (m_display[offset] && value); // If the pixel was alredy on --> CONFLICT
			m_display[offset] = (m_display[offset] ^ value); // Paint sprite with XOR
			break;
		case Display::MSB_PLANE:
			conflict = (m_secondPlane[offset] && value); // If the pixel was alredy on --> CONFLICT
			m_secondPlane[offset] = (m_secondPlane[offset] ^ value); // Paint sprite with XOR
			break;
		default:
			printf("Not valid plane selected! Selected: %d\n", m_activePlane);
			exit(-1);
			break;
		}
	}

	m_needsUpdate = true;
	return conflict;

}

bool Display::IsWindowClose() const
{
	return m_closeWindow;
}


