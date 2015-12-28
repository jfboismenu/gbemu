#pragma once

#include <audio/channelBase.h>
#include <audio/common.h>
#include <common/register.h>

namespace gbemu {

    class Clock;

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

    class WaveSoundEvent: public SoundEventBase
    {
        using SoundEventBase::SoundEventBase;
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
    private:

        char computeSample(float frequency, float timeSinceNoteStart, float duty) const;

        Register< SoundLength >   _rSoundLength;
        Register< Volume >        _rVolume;
        Register< FrequencyLoBits, 0x0, 0xFF >                 _rFrequencyLo;
        Register< FrequencyHiBits, 0x40, 0XFF >                _rFrequencyHiPlayback;
    };
}
