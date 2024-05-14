#include "Audio.h"

Audio::Audio()
{
    m_audioSpec.freq = 440;
    m_audioSpec.format = AUDIO_S16LSB;
    m_audioSpec.channels = 1;
    m_audioSpec.samples = 4096;
    m_audioSpec.userdata = &m_phase;
    //m_audioSpec.callback = AudioCallback;
}

bool Audio::Init()
{
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    if (SDL_OpenAudio(&m_audioSpec, NULL) < 0) {
        printf("Failed to open audio device: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

void Audio::Close()
{
    SDL_CloseAudio();
    SDL_Quit();
}

// Audio callback function to fill audio buffer with samples
void AudioCallback(void* userdata, Uint8* stream, int len) {
    // Cast userdata to double* to use as a sample phase
    double* phase = (double*)userdata;

    // Convert stream to Sint16* for easier sample manipulation
    Sint16* samples = (Sint16*)stream;

    // Number of samples to fill in the buffer
    int sampleCount = len / sizeof(Sint16);

    // Fill the audio buffer with a sine wave at the specified frequency
    for (int i = 0; i < sampleCount; i++) {
        // Calculate the sine wave sample
        double value = sin((*phase) * 2.0 * M_PI);

        // Convert the value to an audio sample and store it in the buffer
        samples[i] = (Sint16)(value * 28000);

        // Update the phase
        *phase += (double)440 / 44100;

        // Wrap the phase if necessary
        if (*phase >= 1.0) {
            *phase -= 1.0;
        }
    }
}
