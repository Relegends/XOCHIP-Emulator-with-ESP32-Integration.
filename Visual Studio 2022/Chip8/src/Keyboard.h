#pragma once
#include <string>
#include <vector>
#include <array>
#include <SDL.h>

class Keyboard
{
public:
	Keyboard() = default;
	Keyboard(const std::vector<std::pair<std::string, int>>& m_keyboard);
	~Keyboard() = default;

	void ProcessInput();

	void SavePreviousInputs();

	bool WaitForInput(int* returnValue);

	int ReadKeyboard(const unsigned int position);

private:
	std::vector<std::pair<std::string, int>> m_keyboard;
	std::array<int, 16> m_inputs;
	std::array<int, 16> m_previousInputs;
	int keyPressed;
};
