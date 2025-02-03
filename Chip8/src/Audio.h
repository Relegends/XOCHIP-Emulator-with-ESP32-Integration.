#pragma once
#include <string>
#include "SDL.h"

class Audio
{
public:
	Audio(unsigned int freq, unsigned int sampleRate, unsigned int volume);

	bool Init();
	void Close();

	void PlaySound(bool on);

private:
	static void SquareWaveCallback(void* userdata, uint8_t* stream, int len);

private:
	SDL_AudioSpec m_audioSpec;
	SDL_AudioDeviceID m_audioDev;
	
	bool m_playingSound;

	unsigned int m_frequency;
	unsigned int m_sampleRate;
	int m_volume;

	uint32_t m_runningSampleIndex;
};

