 #pragma once

#include <portaudio.h>

namespace gbemu {

    class Audio
    {
    public:
        using AudioCallback = void(void* output, const unsigned long sampleCount, const int rate, void* userData);

        Audio(
            const int samples_per_second,
            void* userData,
            AudioCallback userCallback
        );

        ~Audio();

        void start();
        void stop();

    private:
        static int callback(
            const void * input,
            void *raw_output,
            const unsigned long sampleCount,
            const PaStreamCallbackTimeInfo *timeInfo,
            const PaStreamCallbackFlags statusFlags,
            void *userData
        );
        void* _userData;
        AudioCallback* _userCallback;
        PaStream *_stream;
        int _rate;
    };
}
