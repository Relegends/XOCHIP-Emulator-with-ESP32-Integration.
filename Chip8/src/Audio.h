#pragma once
#include <string>
#include <vector>
#include <cmath>
#include "SDL.h"

class Audio
{
public:
	Audio(unsigned int freq, unsigned int sampleRate, unsigned int volume);

	bool Init();
	void Close();

	void PlaySound(bool on);

	void SetPitch(const uint8_t value);
	void LoadPatternBuffer(const uint8_t* source);

private:
	static void SquareWaveCallback(void* userdata, uint8_t* stream, int len);

	static void XOChipAudioCallback(void* userdata, uint8_t* stream, int len);

private:
	SDL_AudioSpec m_audioSpec;
	SDL_AudioDeviceID m_audioDev;
	
	bool m_playingSound;

	unsigned int m_frequency;
	unsigned int m_sampleRate;
	int m_volume;

	uint8_t m_pitch;
	std::vector<uint8_t> m_patternBuffer;

	float m_runningSampleIndex;
};

