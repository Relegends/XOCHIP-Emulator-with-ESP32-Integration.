#pragma once
#include <string>
#include <memory>
#include <FisicalPanel.h>
#include "Log.h"

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

	Display();

	bool Init(
		const unsigned int windowWidth, const unsigned int windowHeight,
		const unsigned int displayWidth, const unsigned int displayHeight,
		const uint16_t backgroundColor, const uint16_t foreground1Color,
		const uint16_t foreground2Color, const uint16_t blendedColor,
		const unsigned int panelWidth, const unsigned int panelHeight,
		const unsigned int panelRotation, const bool rgbOrder,
		const bool isFlipY
	);
	void Update();
	void Close();

	void CLS();	// Clear Screen

	void SwapBuffers();

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

	FisicalPanel& GetPanel() { return m_panel; }

private:
	void UpdateScale();

private:
	void Render();
	void DrawPixel(unsigned int x, unsigned int y, bool planeLSB, bool planeMSB);
	void DrawPixelToCombineBuffer(unsigned int x, unsigned int y, bool planeLSB, bool planeMSB);
	void UpdateTexture();
private:
	unsigned int m_windowHeight;
	unsigned int m_windowWidth;

	uint16_t m_backgroundColor;
	uint16_t m_foreground_1Color;
	uint16_t m_foreground_2Color;
	uint16_t m_blendedColor;

	bool m_closeWindow;
	bool m_needsUpdate;

	unsigned int m_displayWidth;
	unsigned int m_displayHeight;

	bool m_extendedMode;
	bool m_isFlipY;

	float m_scaleX;
	float m_scaleY;

	std::vector<bool> m_display;
	std::vector<bool> m_secondPlane;

	lgfx::LGFX_Sprite m_sprite;

	FisicalPanel m_panel;

	enum ACTIVEPLANE m_activePlane;

	enum PIXELCOLOR
	{
		BACKGROUND = 0,
		FOREGROUND1,
		FOREGROUND2,
		BLENDED
	};
};

