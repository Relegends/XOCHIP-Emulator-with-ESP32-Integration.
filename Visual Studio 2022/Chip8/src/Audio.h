#pragma once
#include <string>
#include <array>
#include <vector>
#include <cmath>
#include <mutex>
#include "SDL.h"
#include "RingBuffer.h"

class Audio
{
public:
	Audio(unsigned int freq, unsigned int sampleRate, unsigned int volume);

	bool Init();
	void Close();

	void PlaySound(bool on);

	void SetPitch(const uint8_t value);
	void LoadPatternBuffer(const uint8_t* source);
	void GenerateAudioSamples();

private:

	static void SquareWaveCallback(void* userdata, uint8_t* stream, int len);

	static void XOChipAudioCallback(void* userdata, uint8_t* stream, int len);

private:
	SDL_AudioSpec m_audioSpec;
	SDL_AudioDeviceID m_audioDev;
	
	bool m_playingSound;
	bool m_patternBufferInit;

	unsigned int m_samplesPerFrame;

	unsigned int m_frequency;
	unsigned int m_sampleRate;
	int m_volume;

	uint8_t m_pitch;

	double m_runningSampleIndex;

	std::mutex m_audioMutex;

	std::array<uint8_t, 16> m_patternBuffer;
	RingBuffer<float> m_ringBuffer;
};

