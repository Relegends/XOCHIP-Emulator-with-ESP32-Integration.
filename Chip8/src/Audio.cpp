#include "Audio.h"

Audio::Audio(unsigned int freq, unsigned int sampleRate, unsigned int volume)
{
    m_frequency = freq;
    m_sampleRate = sampleRate;
    m_volume = volume;

    m_playingSound = false;

    m_runningSampleIndex = 0;

    m_audioDev = 0;

    m_audioSpec.freq = 44100; // CD quality
    m_audioSpec.format = AUDIO_S16LSB;
    m_audioSpec.channels = 1;
    m_audioSpec.samples = 512;
    m_audioSpec.userdata = this;
    m_audioSpec.callback = SquareWaveCallback;
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

void Audio::SquareWaveCallback(void* userdata, uint8_t* stream, int len) {
    Audio* audio = (Audio*)userdata;

    int16_t* data = (int16_t*)stream;
    audio->m_runningSampleIndex = 0;
    uint32_t squareWavePeriod = audio->m_sampleRate / audio->m_frequency;
    uint32_t halfSquareWavePeriod = squareWavePeriod / 2;

    for (int i = 0; i < len / 2; i++)
    {
        data[i] = ((audio->m_runningSampleIndex++ / halfSquareWavePeriod) % 2) ?
            audio->m_volume :
            -(audio->m_volume);
    }
}
