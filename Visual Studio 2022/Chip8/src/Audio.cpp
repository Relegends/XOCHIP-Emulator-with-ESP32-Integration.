#include "Audio.h"

Audio::Audio(unsigned int freq, unsigned int sampleRate, unsigned int volume)
{
	m_frequency = freq;
	m_sampleRate = sampleRate;
	m_volume = volume;

	m_playingSound = false;
	m_patternBufferInit = false;

	m_runningSampleIndex = 0;

	m_pitch = 64;

	m_audioDev = 0;

	m_audioSpec.freq = 44100; // CD quality
	m_audioSpec.format = AUDIO_F32LSB;
	m_audioSpec.channels = 1;
	m_audioSpec.samples = 128;
	m_audioSpec.userdata = this;
	m_audioSpec.callback = XOChipAudioCallback;

	m_samplesPerFrame = m_audioSpec.freq / 60;
	m_ringBuffer.Init(m_samplesPerFrame);
}

bool Audio::Init()
{
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	SDL_AudioSpec obtained;
	m_audioDev = SDL_OpenAudioDevice(NULL, 0, &m_audioSpec, &obtained, 0);

	if (!m_audioDev) {
		printf("Failed to open audio device: %s\n", SDL_GetError());
		return false;
	}

	return true;
}

void Audio::Close()
{
	SDL_CloseAudioDevice(m_audioDev);
	SDL_Quit();
}

void Audio::PlaySound(bool on)
{
	m_playingSound = on;
	SDL_PauseAudioDevice(m_audioDev, !m_playingSound);
}

void Audio::SetPitch(const uint8_t value)
{
	m_pitch = value;
}

void Audio::LoadPatternBuffer(const uint8_t* source)
{
	std::scoped_lock lock(m_audioMutex);
	m_patternBufferInit = true;
	std::memcpy(m_patternBuffer.data(), source, m_patternBuffer.size());
}

void Audio::GenerateAudioSamples()
{
	static float sampleBuffer[1024];
	m_runningSampleIndex = 0;

	if (m_patternBufferInit)
	{
		// Calculate the playback rate in Hz based on the pitch register
		float playbackRate = (4000.0f * std::pow(2.0f, (float(m_pitch) - 64.0f) / 48.0f));
		// Calculate the step size
		float step = playbackRate / m_audioSpec.freq;

		for (int i = 0; i < m_samplesPerFrame; i++)
		{
			// Get the current pattern value from the buffer
			int patternIndex = static_cast<int>(m_runningSampleIndex) / 8;
			int bitShift = static_cast<int>(m_runningSampleIndex) % 8;
			uint8_t patternValue = m_patternBuffer[patternIndex];

			// Shift and mask to get the specific bit
			bool bit = (patternValue >> (7 - bitShift)) & 1;

			// Calculate the sample value based on the bit
			float sample = bit ? 0.5f : -0.5f;
			sampleBuffer[i] = sample;

			// Increment the running sample index by the step size
			m_runningSampleIndex += step;

			// Wrap around the running sample index if it exceeds the buffer size
			if (m_runningSampleIndex >= (m_patternBuffer.size() * 8))
			{
				m_runningSampleIndex = std::fmod(m_runningSampleIndex, m_patternBuffer.size() * 8);
			}
		}
	}
	else
	{
		uint32_t squareWavePeriod = m_sampleRate / m_frequency;
		uint32_t halfSquareWavePeriod = squareWavePeriod / 2;

		for (int i = 0; i < m_samplesPerFrame / 2; i++)
		{
			sampleBuffer[i] = (((int)m_runningSampleIndex++ / halfSquareWavePeriod) % 2) ? 0.5f : -0.5f;
		}
	}

	std::scoped_lock lock(m_audioMutex);
	for (unsigned int i = 0; i < m_samplesPerFrame; i++) {
		m_ringBuffer.Push(sampleBuffer[i]);
	}
}

void Audio::XOChipAudioCallback(void* userdata, uint8_t* stream, int len)
{
	Audio* audio = reinterpret_cast<Audio*>(userdata);
	float* data = reinterpret_cast<float*>(stream);
	int numSamples = len / sizeof(float);

	std::scoped_lock lock(audio->m_audioMutex);
	for (int i = 0; i < numSamples; i++) {
		if (!audio->m_ringBuffer.Empty()) {
			data[i] = audio->m_ringBuffer.Pop();
		}
		else {
			data[i] = 0.0f;
		}
	}
}

void Audio::SquareWaveCallback(void* userdata, uint8_t* stream, int len) {
	Audio* audio = (Audio*)userdata;

	int16_t* data = (int16_t*)stream;
	audio->m_runningSampleIndex = 0;
	uint32_t squareWavePeriod = audio->m_sampleRate / audio->m_frequency;
	uint32_t halfSquareWavePeriod = squareWavePeriod / 2;

	for (int i = 0; i < len / 2; i++)
	{
		data[i] = (((int)audio->m_runningSampleIndex++ / halfSquareWavePeriod) % 2) ? 0.5f : -0.5f;
	}
}