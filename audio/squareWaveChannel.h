#pragma once

#include <common/register.h>
#include <base/cyclicCounter.h>
#include <mutex>
#include <array>

namespace gbemu {

    class Clock;

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

    class SoundLengthWavePatternDutyBits
    {
    public:
        float getWaveDutyPercentage() const
        {
            switch( wavePatternDuty ) {
                case 0:
                    return 0.125f;
                case 1:
                    return 0.25f;
                case 2:
                    return 0.50f;
                case 3:
                    return 0.75f;
                default:
                    JFX_MSG_ASSERT( "Wave pattern duty invalid: " << wavePatternDuty );
            }
        }
        float getSoundLength() const
        {
            return (64 - soundLength) * (1.f / 256);
        }
    private:
        unsigned char soundLength : 6;
        unsigned char wavePatternDuty : 2;
    };
    
    class EnveloppeBits
    {
    public:
        bool isAmplifying() const
        {
            return _direction == 1;
        }
        unsigned char sweepLength : 3;
    private:
        unsigned char _direction : 1;
    public:
        unsigned char initialVolume : 4;
    };

    class SquareWaveChannel
    {
    public:
        SquareWaveChannel(
            const Clock& clock,
            unsigned short soundLengthRegisterAddr,
            unsigned short evenloppeRegisterAddr,
            unsigned short frequencyLowRegisterAddr,
            unsigned short frequencyHiRegisterAddr
        );
        void renderAudio(void* output, const unsigned long frameCount, const int rate, const float realTime);
        void writeByte( unsigned short addr, unsigned char value );
        unsigned char readByte( unsigned short addr ) const;
    private:
        void updateEventsQueue(const float audioFrameStartInSeconds);
        void incrementFirstEventIndex();
        struct SoundEvent
        {
            SoundEvent(
                bool pl,
                bool il,
                int64_t ws,
                float wsis,
                float wlis,
                int wf,
                float d,
                char v
            );
            SoundEvent() = default;
            bool isPlaying;
            bool isLooping;
            int waveStart;
            float waveStartInSeconds;
            float waveLengthInSeconds;
            int waveFrequency;
            float waveDuty;
            char waveVolume;
            float waveEndInSeconds() const;
        };

        char computeSample(float frequency, float timeSinceNoteStart, float duty) const;
        short getGbNote() const;

        Register< SoundLengthWavePatternDutyBits, 0xB0, 0xFF > _nr11;
        Register< EnveloppeBits >                              _nr12;
        Register< FrequencyLoBits, 0x0, 0xFF >                 _nr13;
        Register< FrequencyHiBits, 0x40, 0XFF >                _nr14;
        const Clock&                                           _clock;

        const unsigned short _soundLengthRegisterAddr;
        const unsigned short _evenloppeRegisterAddr;
        const unsigned short _frequencyLowRegisterAddr;
        const unsigned short _frequencyHiRegisterAddr;

        enum {BUFFER_SIZE = 32};

        std::array<SoundEvent, BUFFER_SIZE> _soundEvents;

        using BufferIndex = CyclicCounter<BUFFER_SIZE>;

        BufferIndex               _firstEvent;
        BufferIndex               _lastEvent;
        BufferIndex               _playbackLastEvent;
        std::mutex _mutex;
    };
}
