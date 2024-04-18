#include "Keyboard.h"

Keyboard::Keyboard(const std::vector<std::pair<char, int>>& m_keyboard)
	: m_keyboard(m_keyboard)
{
	m_inputs = {0};
}

void Keyboard::ProcessInput()
{
	SDL_PumpEvents();
	
	for (const auto& key : m_keyboard)
		m_inputs[key.second] = 0;
}

void Keyboard::Update(SDL_KeyboardEvent keyboardEvent)
{
	SDL_Keycode keycode = keyboardEvent.keysym.sym;

	const char* keyName = SDL_GetKeyName(keycode);

	if (m_keyboard.find(*keyName) != m_keyboard.end())
	{
		m_keyboard[*keyName] = keyboardEvent.state;
	}
}
