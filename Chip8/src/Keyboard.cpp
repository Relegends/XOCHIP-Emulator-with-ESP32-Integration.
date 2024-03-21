#include "Keyboard.h"

void Keyboard::Update(SDL_KeyboardEvent keyboardEvent)
{
	SDL_Keycode keycode = keyboardEvent.keysym.sym;

	if (m_keyboard.find(keycode) != m_keyboard.end())
	{
		m_keyboard[keycode] = keyboardEvent.state;
	}
}
