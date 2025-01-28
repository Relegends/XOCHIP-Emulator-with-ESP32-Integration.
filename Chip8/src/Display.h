#pragma once
#include <string>
#include <memory>
#include <array>
#include <functional>
#include <SDL.h>

class Display
{
public:
	enum ACTIVEPLANE
	{
		NONE = 0,
		LSB_PLANE,
		MSB_PLANE,
		BOTH
	};

	Display(const std::string& name,
		const unsigned int windowWidth, const unsigned int windowHeight,
		const unsigned int displayWidth, const unsigned int displayHeight,
		const uint32_t backgroundColor, const uint32_t foreground1Color,
		const uint32_t foreground2Color, const uint32_t blendedColor);

	bool Init();
	void Update();
	void Close();

	void CLS();	// Clear Screen

	void ScrollUp(const unsigned int lines);
	void ScrollDown(const unsigned int lines);
	void ScrollRight(const unsigned int lines);
	void ScrollLeft(const unsigned int lines);

	void SetExtendedMode(const bool enable);
	const bool GetExtendedMode() const;
	
	void SetActivePlane(const unsigned int plane);
	const ACTIVEPLANE GetActivePlane() const;

	bool ChangePixelValue(unsigned int x, unsigned int y, unsigned int col, unsigned int row, const bool value, const bool clipping, const ACTIVEPLANE drawingPlaneOnBothMode = NONE); // Returns wheter there was a collision or not

	bool IsWindowClose() const;

private:
	void Render();
	void DrawPixel(unsigned int x, unsigned int y, bool planeLSB, bool planeMSB);

private:
	unsigned int m_windowHeight;
	unsigned int m_windowWidth;

	uint32_t m_backgroundColor;
	uint32_t m_foreground_1Color;
	uint32_t m_foreground_2Color;
	uint32_t m_blendedColor;
	
	bool m_closeWindow;

	unsigned int m_displayWidth;
	unsigned int m_displayHeight;

	bool m_extendedMode;

	std::string m_windowName;

	std::vector<bool> m_display;
	std::vector<bool> m_secondPlane;

	std::unique_ptr < SDL_Window, std::function<void(SDL_Window*)>> m_window;
	std::unique_ptr<SDL_Renderer, std::function<void(SDL_Renderer*)>> m_renderer;

	enum ACTIVEPLANE m_activePlane;

	enum PIXELCOLOR
	{
		BACKGROUND = 0,
		FOREGROUND1,
		FOREGROUND2,
		BLENDED
	};
};

