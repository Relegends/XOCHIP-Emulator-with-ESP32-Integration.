#pragma once
#include <array>
#include <cmath>
#include <mutex>
#include <string>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/i2s_std.h>
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

	// Deprecated callbacks, replaced by AudioTask
	//static void SquareWaveCallback(void* userdata, uint8_t* stream, int len);
	//static void XOChipAudioCallback(void* userdata, uint8_t* stream, int len);

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

	// Internal DAC config
	i2s_port_t m_i2sPort;
	i2s_chan_handle_t m_txHandle;
	TaskHandle_t m_audioTaskHandle;
	int m_bufferDMA;
};

