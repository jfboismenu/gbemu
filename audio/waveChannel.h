#pragma once

#include <audio/channelBase.h>
#include <audio/frequency.h>
#include <audio/common.h>
#include <cpu/registers.h>
#include <common/register.h>

namespace gbemu {

    class PAPUClocks;

    class OnOff
    {
    private:
        unsigned char _unused : 7;
    public:
        unsigned char isOn : 1;
    };

    class SoundLength
    {
    public:
        float getSoundLength() const
        {
            return 1 - _soundLength / 256.f;
        }
    private:
        unsigned char _soundLength;
    };

    class Volume
    {
    public:
        char volumeShift() const
        {
            switch(_volume) {
                case 0: return 4;
                default: return _volume - 1;
            }
        }
    private:
        unsigned char _unused : 5;
        unsigned char _volume : 2;
        unsigned char _unused2 : 1;
    };

    using WavePatternSamples = std::array< unsigned char, kWavePatternRAMEnd - kWavePatternRAMStart >;

    class WaveChannel : public ChannelBase, public Frequency
    {
    public:
        WaveChannel(
            const PAPUClocks& clock,
            std::mutex& mutex
        );
        void writeByte( unsigned short addr, unsigned char value );
        bool contains(unsigned short addr) const;
        void emulate(int64_t currentCycle);

    private:
        // Current step in the played frequency.
        CyclicCounterT<32> _currentSample;
        Register< OnOff >         _rOnOff;
        Register< Volume >        _rVolume;
        WavePatternSamples _wavePattern;
        unsigned char * const _wavePatternPtr;
    };
}
