#include "Keyboard.h"

Keyboard::Keyboard(const std::vector<std::pair<char, int>>& m_keyboard)
	: m_keyboard(m_keyboard)
{
	m_inputs = {0};
	m_previousInputs = m_inputs;
}

void Keyboard::ProcessInput()
{
	SDL_PumpEvents();
	
	for (const auto& key : m_keyboard)
	{
		 SDL_Keycode k = SDL_GetKeyFromName(&key.first);
		 m_inputs[key.second] = *(SDL_GetKeyboardState(&k));
	}
}

void Keyboard::SavePreviousInputs()
{
	m_previousInputs = m_inputs;
}

bool Keyboard::WaitForInput(int* returnValue)
{
	for (int i = 0; i < m_inputs.size(); i++)
	{
		if (m_inputs[i] != m_previousInputs[i])
		{
			m_previousInputs = m_inputs;
			*returnValue = i;
			return true;
		}
	}
	return false;
}

int Keyboard::ReadKeyboard(const unsigned int position)
{
	return m_inputs[position];
}
