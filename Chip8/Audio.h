#pragma once
#include <string>
#include "SDL.h"

class Audio
{
public:
	Audio();

	bool Init();
	void Close();

private:
	//void AudioCallback(void* userdata, Uint8* stream, int len);

private:
	SDL_AudioSpec m_audioSpec;
	float m_phase;
};

