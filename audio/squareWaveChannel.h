#pragma once

#include <audio/common.h>
#include <common/register.h>
#include <base/cyclicCounter.h>
#include <mutex>
#include <array>

namespace gbemu {

    class Clock;

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

        float getSweepLength() const
        {
            return _sweepLength / 64.f;
        }

    private:
        unsigned char _sweepLength : 3;
        unsigned char _direction : 1;
    public:
        unsigned char initialVolume : 4;
    };

    class FrequencySweepBits
    {
    public:
        bool isIncreasing() const
        {
            return _isIncreasing == 0;
        }

        int getNbSweepShifts() const
        {
            return _nbSweepShifts;
        }

        float getSweepLength() const
        {
            return _sweepLength / 128.f;
        }
    private:
        unsigned char _nbSweepShifts: 3;
        unsigned char _isIncreasing: 1;
        unsigned char _sweepLength: 4;
    };

    class SquareWaveChannel
    {
    public:
        SquareWaveChannel(
            const Clock& clock,
            unsigned short frequencyShiftRegisterAddr,
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

        class SoundEvent : public SoundEventBase
        {
        public:
            SoundEvent(
                bool ip,
                bool il,
                int wf,
                int64_t ws,
                float wsis,
                float wlis,
                float d,
                char v,
                bool va,
                float sl
            );
            SoundEvent() = default;
            int waveStart;
            float waveStartInSeconds;
            float waveLengthInSeconds;
            float waveDuty;
            float waveEndInSeconds() const;
            unsigned char getVolumeAt(float currentTime) const;
        private:
            char waveVolume;
            bool isVolumeAmplifying;
            float  sweepLength;
        };

        char computeSample(float frequency, float timeSinceNoteStart, float duty) const;
        short getGbNote() const;

        Register< FrequencySweepBits, 0xFF, 0xFF >             _rFrequencySweep;
        Register< SoundLengthWavePatternDutyBits, 0xB0, 0xFF > _rLengthDuty;
        Register< EnveloppeBits >                              _rEnveloppe;
        Register< FrequencyLoBits, 0x0, 0xFF >                 _rFrequencyLo;
        Register< FrequencyHiBits, 0x40, 0XFF >                _rFrequencyHiPlayback;
        const Clock&                                           _clock;

        const unsigned short _frequencySweepRegisterAddr;
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
        std::mutex                _mutex;
    };
}
