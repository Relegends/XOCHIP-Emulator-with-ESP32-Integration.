#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <mutex>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/dac.h>
#include <driver/i2s.h>
#include "RingBuffer.h"
#include "Log.h"

class Audio
{
public:
	Audio();
	~Audio();

	bool Init(unsigned int freq, unsigned int sampleRate, unsigned int volume);
	void Close();

	void PlaySound(bool on);
	void SetPitch(const uint8_t value);
	void LoadPatternBuffer(const uint8_t* source);
	void GenerateAudioSamples();

private:
	static void XOAudioTask(void* param);

private:
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

	i2s_port_t m_i2sPort;
	TaskHandle_t m_audioTaskHandle;
};
