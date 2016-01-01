#pragma once

#include <base/cyclicCounter.h>
#include <mutex>
#include <array>

namespace gbemu {

    class Clock;

    template< typename Derived, typename SoundEventType >
    class ChannelBase
    {
    public:
        void updateEventsQueue(const float audioFrameStartInSeconds);
        void renderAudio(
            void* raw_output,
            const unsigned long frameCount,
            const int rate,
            const float realTime
        );
    protected:
        void insertEvent(const SoundEventType& event);
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
