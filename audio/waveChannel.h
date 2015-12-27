#pragma once

#include <audio/common.h>
#include <common/register.h>
#include <base/cyclicCounter.h>
#include <mutex>
#include <array>

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

    class WaveChannel
    {
    public:
        WaveChannel(
            const Clock& clock
        );
        void renderAudio(void* output, const unsigned long frameCount, const int rate, const float realTime);
        void writeByte( unsigned short addr, unsigned char value );
        unsigned char readByte( unsigned short addr ) const;
    private:
        void updateEventsQueue(const float audioFrameStartInSeconds);
        void incrementFirstEventIndex();

        char computeSample(float frequency, float timeSinceNoteStart, float duty) const;

        class SoundEvent: public SoundEventBase
        {
            using SoundEventBase::SoundEventBase;
        };

        Register< SoundLength >   _rSoundLength;
        Register< Volume >        _rVolume;
        Register< FrequencyLoBits, 0x0, 0xFF >                 _rFrequencyLo;
        Register< FrequencyHiBits, 0x40, 0XFF >                _rFrequencyHiPlayback;
        const Clock&                                           _clock;

        enum {BUFFER_SIZE = 32};

        std::array<SoundEvent, BUFFER_SIZE> _soundEvents;

        using BufferIndex = CyclicCounter<BUFFER_SIZE>;

        BufferIndex               _firstEvent;
        BufferIndex               _lastEvent;
        BufferIndex               _playbackLastEvent;
        std::mutex                _mutex;
    };
}
