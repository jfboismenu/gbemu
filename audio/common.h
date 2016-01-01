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

    struct WaveChannelStateBase
    {
        WaveChannelStateBase(
            bool il,
            float wf,
            int64_t ws,
            float wsis,
            float wlis
        );
        WaveChannelStateBase() = default;
        float waveEndInSeconds() const;

        bool isLooping;
        float waveFrequency;
        int waveStart;
        float waveStartInSeconds;
        float waveLengthInSeconds;

        float timeStamp;
    };
}
