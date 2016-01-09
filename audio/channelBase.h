#pragma once

#include <base/cyclicCounter.h>
#include <audio/common.h>
#include <mutex>
#include <array>

namespace gbemu {

    class CPUClock;
    class PAPUClocks;

    class ChannelBase
    {
    public:
        void renderAudio(
            void* raw_output,
            const unsigned long frameCount,
            const int rate,
            const int64_t cpuClock
        );
        void updateEventsQueue(int64_t currentTime);
    protected:
        void insertEvent(
            int64_t time,
            char sample
        );
        ChannelBase(
            const PAPUClocks& clocks,
            std::mutex& mutex
        );

        const PAPUClocks& _clocks;

        enum {BUFFER_SIZE = 131092};

        std::array<SoundEvent, BUFFER_SIZE> _soundEvents;

        using BufferIndex = CyclicCounterT<BUFFER_SIZE>;

        BufferIndex               _firstEvent;
        BufferIndex               _lastEvent;
        std::mutex&               _mutex;
    };
}
