#include "audio.h"
#include <stdexcept>
#include <iostream>

namespace gbemu {

    Audio::Audio(
        const int rate,
        AudioCallback user_callback
    ) : _user_callback(user_callback),
        _rate(rate)
    {
        PaError err = Pa_Initialize();
        if( err != paNoError ) {
            throw std::runtime_error(Pa_GetErrorText(err));
        }

        // Open an audio I/O stream.
        err = Pa_OpenDefaultStream(
            &_stream,
            0,          // no input channels
            1,          // mono output
            paInt8,     // 8 bit integer output
            rate,       // sample rate
            1470,        // number of frames asked each iteration.
            Audio::callback, // this is your callback function
            this
        );
        if( err != paNoError ) {
            throw std::runtime_error(Pa_GetErrorText(err));
        }
    }

    void Audio::start()
    {
        Pa_StartStream(_stream);
    }

    void Audio::stop()
    {
        Pa_StopStream(_stream);
    }

    Audio::~Audio()
    {
        const PaError err = Pa_Terminate();
        if( err != paNoError ) {
            throw std::runtime_error(Pa_GetErrorText(err));
        }
    }

    int Audio::callback(
        const void * input,
        void *raw_output,
        const unsigned long frameCount,
        const PaStreamCallbackTimeInfo *timeInfo,
        const PaStreamCallbackFlags statusFlags,
        void *userData
    )
    {
        Audio& audio(*reinterpret_cast<Audio*>(userData));
        (audio._user_callback)(
            raw_output,
            frameCount,
            audio._rate
        );
        return paContinue;
    }


};
