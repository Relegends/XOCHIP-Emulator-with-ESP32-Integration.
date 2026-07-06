#include "Audio.h"

Audio::Audio()
{
	m_frequency = 0;
	m_sampleRate = 0;
	m_volume = 0;

	m_playingSound = false;
	m_patternBufferInit = false;

	m_runningSampleIndex = 0.0;
	m_pitch = 64;
	m_samplesPerFrame = 0;

	m_i2sPort = I2S_NUM_0;
	m_audioTaskHandle = nullptr;
}

Audio::~Audio()
{
	Close();
}

bool Audio::Init(unsigned int freq, unsigned int sampleRate, unsigned int volume)
{
	LOG("[Audio] Initializing audio...");

	m_frequency = freq;
	m_sampleRate = sampleRate;
	m_volume = volume;
	m_samplesPerFrame = m_sampleRate / 60;
	m_ringBuffer.Init(m_samplesPerFrame * 2);

	// Legacy I2S driver required for built-in DAC on GPIO 25/26.
	// The new i2s_std driver drives those pins as digital I2S, bypassing the DAC hardware.
	i2s_config_t i2sConfig = {};
	i2sConfig.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN);
	i2sConfig.sample_rate = m_sampleRate;
	i2sConfig.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
	i2sConfig.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
	i2sConfig.communication_format = I2S_COMM_FORMAT_STAND_MSB;
	i2sConfig.intr_alloc_flags = 0;
	i2sConfig.dma_buf_count = 8;
	i2sConfig.dma_buf_len = 64;
	i2sConfig.use_apll = false;
	i2sConfig.tx_desc_auto_clear = true;

	esp_err_t err = i2s_driver_install(m_i2sPort, &i2sConfig, 0, nullptr);
	if (err != ESP_OK) {
		LOGE("[Audio] Failed to install I2S driver: %s", esp_err_to_name(err));
		return false;
	}

	err = i2s_set_dac_mode(I2S_DAC_CHANNEL_BOTH_EN);
	if (err != ESP_OK) {
		LOGE("[Audio] Failed to set DAC mode: %s", esp_err_to_name(err));
		i2s_driver_uninstall(m_i2sPort);
		return false;
	}

	// Re-enable DAC explicitly: CONFIG_ADC_DISABLE_DAC=y means any ADC2 use silently
	// kills the DAC output on GPIO 25/26. This overrides that.
	dac_output_enable(DAC_CHANNEL_1); // GPIO 25
	dac_output_enable(DAC_CHANNEL_2); // GPIO 26

	if (xTaskCreate(XOAudioTask, "XOAudioTask", 4096, this, configMAX_PRIORITIES - 1, &m_audioTaskHandle) != pdPASS) {
		LOGE("[Audio] Failed to create audio task");
		i2s_driver_uninstall(m_i2sPort);
		return false;
	}

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
	LOG("[Audio] Audio closed");
}

void Audio::PlaySound(bool on)
{
	if (m_playingSound == on) return;
	m_playingSound = on;
	i2s_set_dac_mode(on ? I2S_DAC_CHANNEL_BOTH_EN : I2S_DAC_CHANNEL_DISABLE);
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

	if (m_patternBufferInit)
	{
		float playbackRate = (4000.0f * std::pow(2.0f, (float(m_pitch) - 64.0f) / 48.0f));
		float step = playbackRate / 44100.0f;

		for (unsigned int i = 0; i < m_samplesPerFrame; i++)
		{
			int patternIndex = static_cast<int>(m_runningSampleIndex) / 8;
			int bitShift = static_cast<int>(m_runningSampleIndex) % 8;
			uint8_t patternValue = m_patternBuffer[patternIndex];
			bool bit = (patternValue >> (7 - bitShift)) & 1;
			sampleBuffer[i] = bit ? 0.5f : -0.5f;

			m_runningSampleIndex += step;
			if (m_runningSampleIndex >= (m_patternBuffer.size() * 8))
				m_runningSampleIndex = std::fmod(m_runningSampleIndex, m_patternBuffer.size() * 8);
		}
	}
	else
	{
		uint32_t squareWavePeriod = m_sampleRate / m_frequency;
		uint32_t halfSquareWavePeriod = squareWavePeriod / 2;
		for (unsigned int i = 0; i < m_samplesPerFrame; i++)
			sampleBuffer[i] = (((int)m_runningSampleIndex++ / halfSquareWavePeriod) % 2) ? 0.5f : -0.5f;
	}

	std::scoped_lock lock(m_audioMutex);
	for (unsigned int i = 0; i < m_samplesPerFrame; i++)
		m_ringBuffer.Push(sampleBuffer[i]);
}

void Audio::XOAudioTask(void *param)
{
	Audio* audio = static_cast<Audio*>(param);
	const size_t MONO_SAMPLES = 256;
	uint16_t stereoBuffer[MONO_SAMPLES * 2];

	while (true) {
		size_t numMono = 0;

		if (audio->m_playingSound) {
			std::scoped_lock lock(audio->m_audioMutex);
			while (!audio->m_ringBuffer.Empty() && numMono < MONO_SAMPLES) {
				float s = audio->m_ringBuffer.Pop();
				float sv = std::clamp(s * audio->m_volume / 100.0f, -0.5f, 0.5f);
				uint8_t dacByte = (uint8_t)((sv + 0.5f) * 255.0f);
				uint16_t dacWord = (uint16_t)dacByte << 8;
				stereoBuffer[numMono * 2 + 0] = dacWord;
				stereoBuffer[numMono * 2 + 1] = dacWord;
				numMono++;
			}
		}

		for (size_t i = numMono; i < MONO_SAMPLES; i++) {
			stereoBuffer[i * 2 + 0] = 0x8000;
			stereoBuffer[i * 2 + 1] = 0x8000;
		}

		size_t bytesWritten = 0;
		i2s_write(audio->m_i2sPort, stereoBuffer, sizeof(stereoBuffer), &bytesWritten, portMAX_DELAY);
	}
}
