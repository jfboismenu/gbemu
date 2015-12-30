#pragma once

#include <base/cyclicCounter.h>
#include <mutex>
#include <array>

namespace gbemu {

    class Clock;

    template< typename SoundEventType >
    class ChannelBase
    {
    public:
        void updateEventsQueue(const float audioFrameStartInSeconds);
    protected:
        ChannelBase(const Clock& clock);

        const Clock&                                           _clock;

        enum {BUFFER_SIZE = 512};

        std::array<SoundEventType, BUFFER_SIZE> _soundEvents;

        using BufferIndex = CyclicCounter<BUFFER_SIZE>;

        BufferIndex               _firstEvent;
        BufferIndex               _lastEvent;
        BufferIndex               _playbackLastEvent;
        std::mutex                _mutex;
    };
}
