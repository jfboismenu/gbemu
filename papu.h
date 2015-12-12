#pragma once

#include "register.h"
#include <array>

namespace gbemu {

    class Clock;

    class CyclicCounter
    {
    public:
        CyclicCounter(const int cycleLength);
        CyclicCounter& operator++();
        CyclicCounter operator+(int i) const;
        CyclicCounter operator-(int i) const;
        bool operator!=(const CyclicCounter&) const;
        bool operator==(const CyclicCounter&) const;
        operator int() const;
    private:
        const int _cycleLength;
        int _count;
    };

    class PAPU
    {
    public:
        static void renderAudio(void* output, const unsigned long frameCount, const int rate, void* userData);
        PAPU( const Clock& clock );
        void writeByte( unsigned short addr, unsigned char value );
        unsigned char readByte( unsigned short addr ) const;
    private:
        void renderAudioInternal(void* output, const unsigned long frameCount, const int rate);

        class NR52bits
        {
        public:
            JFX_INLINE unsigned char readUnused() const
            {
                return static_cast< unsigned char >( _unused << 4 );
            }
            unsigned char _sound1On : 1;
            unsigned char _sound2On : 1;
            unsigned char _sound3On : 1;
            unsigned char _sound4On : 1;
        private:
            unsigned char _unused : 3;
        public:
            unsigned char _allSoundOn : 1;
        };

        class FrequencyLoBits
        {
        public:
            unsigned char _freqLo;
        };

        class FrequencyHiBits
        {
        public:
            unsigned char _freqHi : 3;
        private:
            unsigned char _unused : 3;
            unsigned char _consecutive : 1;
        public:
            JFX_INLINE bool isLooping() const
            {
                return _consecutive == 0;
            }
            unsigned char _initialize : 1;
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

        class SoundLengthBits : private SoundLengthWavePatternDutyBits
        {
        public:
            unsigned char getSoundLength()
            {
                return SoundLengthWavePatternDutyBits::getSoundLength();
            }
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
        class NoiseEnveloppeBits : private EnveloppeBits
        {
        public:
            unsigned char getInitialVolume()
            {
                return initialVolume;
            }
            bool isAmplifying() const
            {
                return EnveloppeBits::isAmplifying();
            }
        };
        class MainVolumeOutputControlBits
        {
        public:
            unsigned char rightMainOutputLevel : 3;
            unsigned char outputVinToRightTerminal : 1;
            unsigned char leftMainOutputLevel : 3;
            unsigned char outputVinToLeftTerminal : 1;
        };
        class SoundOutputTerminalSelect
        {
        public:
            unsigned char channel1Right : 1;
            unsigned char channel2Right : 1;
            unsigned char channel3Right : 1;
            unsigned char channel4Right : 1;
            unsigned char channel1Left : 1;
            unsigned char channel2Left : 1;
            unsigned char channel3Left : 1;
            unsigned char channel4Left : 1;
        };

        bool isRegisterAvailable( const unsigned short addr ) const;

        class SquareWaveChannel
        {
        public:
            SquareWaveChannel( const Clock& clock );
            void renderAudio(void* output, const unsigned long frameCount, const int rate);
            void writeByte( unsigned short addr, unsigned char value );
            unsigned char readByte( unsigned short addr ) const;
        private:
            void updatePlaybackInterval();
            void updateEventsQueue(const float audioFrameStartInSeconds);
            void incrementFirstEventIndex();
            struct SoundEvent
            {
                SoundEvent(
                    bool il,
                    int64_t ws,
                    float wsis,
                    float wlis,
                    int wf
                );
                SoundEvent() = default;
                bool isLooping;
                int waveStart;
                float waveStartInSeconds;
                float waveLengthInSeconds;
                int waveFrequency;
                float waveEndInSeconds() const;
            };

            char computeSample(float frequency, float timeSinceNoteStart) const;
            short getGbNote() const;

            Register< SoundLengthWavePatternDutyBits, 0xB0, 0xFF > _nr11;
            Register< EnveloppeBits >                              _nr12;
            Register< FrequencyLoBits, 0x0, 0xFF >                 _nr13;
            Register< FrequencyHiBits, 0x40, 0XFF >                _nr14;
            const Clock&                                           _clock;
            int64_t _playbackIntervalEnd;
            int64_t _playbackIntervalStart;

            std::array<SoundEvent, 32> _soundEvents;

            CyclicCounter                _firstEvent;
            CyclicCounter                _lastEvent;

        } _squareWaveChannel;

        Register< EnveloppeBits > _nr32;
        Register< SoundLengthBits, 0xB0, 0xFF > _nr41;
        Register< NoiseEnveloppeBits > _nr42;

        Register< MainVolumeOutputControlBits > _nr50;
        Register< SoundOutputTerminalSelect > _nr51;
        Register< NR52bits, 0xFF, 0xF0 > _nr52;

        const Clock& _clock;
    };

}
