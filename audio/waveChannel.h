#pragma once

#include <audio/channelBase.h>
#include <audio/common.h>
#include <cpu/registers.h>
#include <common/register.h>

namespace gbemu {

    class Clock;

    class OnOff
    {
    public:
        bool isOn() const
        {
            return _onOff == 1;
        }
    private:
        unsigned char _unused : 7;
        unsigned char _onOff : 1;
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
        char getVolumeShift() const
        {
            switch(_volume) {
                case 0: return -1;
                default: return _volume - 1;
            }
        }
    private:
        unsigned char _unused : 5;
        unsigned char _volume : 2;
        unsigned char _unused2 : 1;
    };

    using WavePatternSamples = std::array< char, kWavePatternRAMEnd - kWavePatternRAMStart >;

    class WaveSoundEvent: public SoundEventBase
    {
    public:
        WaveSoundEvent() = default;
        WaveSoundEvent(
            bool ip,
            bool il,
            int wf,
            int64_t ws,
            float wsis,
            float wlis,
            const WavePatternSamples& samples
        );
    private:
        WavePatternSamples _samples;
    };

    class WaveChannel : public ChannelBase<WaveSoundEvent>
    {
    public:
        WaveChannel(
            const Clock& clock
        );
        void renderAudio(void* output, const unsigned long frameCount, const int rate, const float realTime);
        void writeByte( unsigned short addr, unsigned char value );
        unsigned char readByte( unsigned short addr ) const;
        bool contains(unsigned short addr) const;
    private:
        short getGbNote() const;
        char computeSample(float frequency, float timeSinceNoteStart, float duty) const;

        Register< OnOff >         _rOnOff;
        Register< SoundLength >   _rSoundLength;
        Register< Volume >        _rVolume;
        Register< FrequencyLoBits, 0x0, 0xFF >                 _rFrequencyLo;
        Register< FrequencyHiBits, 0x40, 0XFF >                _rFrequencyHiPlayback;

        WavePatternSamples _wavePattern;
        char * const _wavePatternPtr;
    };
}
