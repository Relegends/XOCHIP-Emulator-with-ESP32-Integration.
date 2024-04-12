#include "Keyboard.h"

void Keyboard::Update(SDL_KeyboardEvent keyboardEvent)
{
	SDL_Keycode keycode = keyboardEvent.keysym.sym;

	const char* keyName = SDL_GetKeyName(keycode);

	if (m_keyboard.find(*keyName) != m_keyboard.end())
	{
		m_keyboard[*keyName] = keyboardEvent.state;
	}
}
