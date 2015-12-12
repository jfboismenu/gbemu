#pragma once

#include <portaudio.h>

namespace gbemu {

    class Audio
    {
    public:
        using AudioCallback = void(void* output, const unsigned long frameCount, const int rate);

        Audio(
            const int samples_per_second,
            AudioCallback user_callback
        );

        ~Audio();

        void start();
        void stop();

    private:
        static int callback(
            const void * input,
            void *raw_output,
            const unsigned long frameCount,
            const PaStreamCallbackTimeInfo *timeInfo,
            const PaStreamCallbackFlags statusFlags,
            void *userData
        );
        AudioCallback* _user_callback;
        PaStream *_stream;
        int _rate;
    };
}
