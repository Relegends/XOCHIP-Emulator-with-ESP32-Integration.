#include "Keyboard.h"

Keyboard::Keyboard(const std::vector<std::pair<std::string, int>>& m_keyboard)
	: m_keyboard(m_keyboard)
{
	m_inputs = {0};
	m_previousInputs = m_inputs;
	keyPressed = -1;
}

void Keyboard::ProcessInput()
{
	SDL_PumpEvents();
	const uint8_t* keyBoardState = SDL_GetKeyboardState(NULL);

	for (const auto& key : m_keyboard)
	{	 
		 m_inputs[key.second] = keyBoardState[SDL_GetScancodeFromName(key.first.c_str())];
	}
}

void Keyboard::SavePreviousInputs()
{
	m_previousInputs = m_inputs;
}

bool Keyboard::WaitForInput(int* returnValue)
{
	if (keyPressed == -1)
	{
		for (int i = 0; i < m_inputs.size(); i++)
		{
			if (m_inputs[i] != m_previousInputs[i])
			{
				m_previousInputs = m_inputs;
				keyPressed = i;
				return false;
			}
		}
	}
	else
	{
		if (m_inputs[keyPressed])
			return false;
		else
		{
			*returnValue = keyPressed;
			keyPressed = -1;
			return true;
		}
	}
	return false;
}

int Keyboard::ReadKeyboard(const unsigned int position)
{
	return m_inputs[position];
}
