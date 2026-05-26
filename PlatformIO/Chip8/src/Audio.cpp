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

	m_samplesPerFrame = m_sampleRate / 60;
	m_ringBuffer.Init(m_samplesPerFrame * 2);

	m_i2sPort = I2S_NUM_0;
	m_txHandle = nullptr;
	m_bufferDMA = 256;
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

	i2s_chan_config_t channelConfig = I2S_CHANNEL_DEFAULT_CONFIG(m_i2sPort, I2S_ROLE_MASTER);
	channelConfig.auto_clear = true;
	
	esp_err_t err = i2s_new_channel(&channelConfig, &m_txHandle, nullptr);
	if (err != ESP_OK) {
		LOGE("[Audio] Failed to create I2S channel: %s", esp_err_to_name(err));
		return false;
	}

	i2s_std_config_t i2sConfig = {
		.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(m_sampleRate),
		.slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
		.gpio_cfg =
		{
			.mclk = I2S_GPIO_UNUSED,
			.bclk = I2S_GPIO_UNUSED,
			.ws = I2S_GPIO_UNUSED,
			.dout = I2S_GPIO_UNUSED,
			.din = I2S_GPIO_UNUSED,
			.invert_flags =
			{
				.mclk_inv = false,
				.bclk_inv = false,
				.ws_inv = false
			}
		}
	};


	err = i2s_channel_init_std_mode(m_txHandle, &i2sConfig);
	if (err != ESP_OK) {
		LOGE("[Audio] Failed to init I2S channel: %s", esp_err_to_name(err));
		return false;
	}

	err = i2s_channel_enable(m_txHandle);
	if (err != ESP_OK) {
		LOGE("[Audio] Failed to enable I2S channel: %s", esp_err_to_name(err));
		return false;
	}

	if (xTaskCreate(XOAudioTask, "XOAudioTask", 4096, this, configMAX_PRIORITIES - 1, &m_audioTaskHandle) != pdPASS) {
		LOGE("[Audio] Failed to create audio task");
		i2s_channel_disable(m_txHandle);
		i2s_del_channel(m_txHandle);
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
	if (m_txHandle) {
		i2s_channel_disable(m_txHandle);
		i2s_del_channel(m_txHandle);
		m_txHandle = nullptr;
	}

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
			i2s_channel_write(audio->m_txHandle, sampleBuffer, sizeof(sampleBuffer), nullptr, portMAX_DELAY);
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
		
		i2s_channel_write(audio->m_txHandle, sampleBuffer, sizeof(sampleBuffer), nullptr, portMAX_DELAY);
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