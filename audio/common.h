#pragma once

#include <common/common.h>

namespace gbemu {
    class FrequencyLoBits
    {
    public:
        unsigned char freqLo;
    };

    class FrequencyHiBits
    {
    public:
        unsigned char freqHi : 3;
    private:
        unsigned char _unused : 3;
        unsigned char _consecutive : 1;
    public:
        JFX_INLINE bool isLooping() const
        {
            return _consecutive == 0;
        }
        unsigned char initialize : 1;
    };

    struct SoundEventBase
    {
        SoundEventBase(
            bool ip,
            bool il,
            int wf
        );
        SoundEventBase() = default;
        bool isPlaying;
        bool isLooping;
        int waveFrequency;
    };
}
