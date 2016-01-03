#pragma once

#include <base/cyclicCounter.h>
#include <audio/common.h>
#include <mutex>
#include <array>

namespace gbemu {

    class Clock;

    class ChannelBase
    {
    public:
        void renderAudio(
            void* raw_output,
            const unsigned long frameCount,
            const int rate,
            const float realTime
        );
    protected:
        ChannelBase(
            const Clock& clock,
            std::mutex& mutex
        );

        const Clock&                                           _clock;

        enum {BUFFER_SIZE = 512};

        std::array<SoundEvent, BUFFER_SIZE> _soundEvents;

        using BufferIndex = CyclicCounter<BUFFER_SIZE>;

        BufferIndex               _firstEvent;
        BufferIndex               _lastEvent;
        BufferIndex               _playbackLastEvent;
        std::mutex&               _mutex;
    };
}
