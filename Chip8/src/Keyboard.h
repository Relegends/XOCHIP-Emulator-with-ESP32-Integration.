#pragma once
#include <unordered_map>
#include <SDL.h>

class Keyboard
{
public:
	Keyboard() = default;
	Keyboard(const std::unordered_map<char, bool>& m_keyboard) : m_keyboard(m_keyboard) {}
	~Keyboard() = default;

	void Update(SDL_KeyboardEvent keyboardEvent);

private:
	std::unordered_map<char, bool> m_keyboard;

};
