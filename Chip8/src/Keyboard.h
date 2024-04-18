#pragma once
#include <vector>
#include <array>
#include <SDL.h>

class Keyboard
{
public:
	Keyboard() = default;
	Keyboard(const std::vector<std::pair<char, int>>& m_keyboard);
	~Keyboard() = default;

	void ProcessInput();
	void Update(SDL_KeyboardEvent keyboardEvent);

private:
	std::vector<std::pair<char, int>> m_keyboard;
	std::array<int, 16> m_inputs;
};
