#include "Audio.h"

Audio::Audio(unsigned int freq, unsigned int sampleRate, unsigned int volume)
{
	m_frequency = freq;
	m_sampleRate = sampleRate;
	m_volume = volume;

	m_playingSound = false;
	m_patternBufferInit = false;

	m_runningSampleIndex = 0.0;

	m_pitch = 64;

	m_samplesPerFrame = m_sampleRate / 60;
	m_ringBuffer.Init(m_samplesPerFrame * 2);

	m_i2sPort = I2S_NUM_0;
	m_bufferDMA = 256;
}

Audio::~Audio()
{
	Close();
}

bool Audio::Init()
{
	i2s_config_t i2sConfig = {};
	i2sConfig.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN);
	i2sConfig.sample_rate = m_sampleRate;
	i2sConfig.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
	i2sConfig.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
	i2sConfig.communication_format = I2S_COMM_FORMAT_I2S_MSB;
	i2sConfig.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
	i2sConfig.dma_buf_len = m_bufferDMA;
	i2sConfig.dma_buf_count = 4;

	esp_err_t err = i2s_driver_install(m_i2sPort, &i2sConfig, 0, nullptr);
	if (err != ESP_OK) {
		LOGE("[Audio] Failed to install I2S driver: %d", err);
		return false;
	}

	i2s_set_dac_mode(I2S_DAC_CHANNEL_BOTH_EN);
	i2s_set_pin(m_i2sPort, nullptr);

	xTaskCreate(XOAudioTask, "XOAudioTask", 2048, this, configMAX_PRIORITIES - 1, &m_audioTaskHandle);
	LOG("[Audio] Audio initialized successfully");

	return true;
}

void Audio::Close()
{
	if (m_audioTaskHandle) {
		vTaskDelete(m_audioTaskHandle);
		m_audioTaskHandle = nullptr;
	}

	i2s_driver_uninstall(m_i2sPort);
	LOG("[Audio] Audio closed successfully");
}

void Audio::PlaySound(bool on)
{
	m_playingSound = on;
}

void Audio::SetPitch(const uint8_t value)
{
	m_pitch = value;
}

void Audio::LoadPatternBuffer(const uint8_t* source)
{
	std::scoped_lock lock(m_audioMutex);
	m_patternBufferInit = true;
	memcpy(m_patternBuffer.data(), source, m_patternBuffer.size());
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
		float step = playbackRate / 44100.0f;

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

void Audio::XOAudioTask(void *param)
{
	Audio* audio = static_cast<Audio*>(param);
	const size_t bufferSize = 256; // SAME AS m_bufferDMA
	int16_t sampleBuffer[bufferSize];

	while (true) {
		if (!audio->m_playingSound) {
			memset(sampleBuffer, 0, sizeof(sampleBuffer));
			i2s_write(audio->m_i2sPort, sampleBuffer, sizeof(sampleBuffer), nullptr, portMAX_DELAY);
			continue;
		}

		size_t numSamples = 0;
		
		std::scoped_lock lock(audio->m_audioMutex);
		while (!audio->m_ringBuffer.Empty() && numSamples < bufferSize) {
			float sample = audio->m_ringBuffer.Pop();
			int16_t val = static_cast<int16_t>(sample * 32767.0f * 2.0f);
			val = (val * audio->m_volume) / 100;
			sampleBuffer[numSamples++] = val;
		}

		if (numSamples < bufferSize)
			memset(&sampleBuffer[numSamples], 0, (bufferSize - numSamples) * sizeof(int16_t));
		
		i2s_write(audio->m_i2sPort, sampleBuffer, sizeof(sampleBuffer), nullptr, portMAX_DELAY);
	}
}

/*
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
*/