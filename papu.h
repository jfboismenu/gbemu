#pragma once

#include "register.h"

namespace gbemu {

    class Clock;

    class SoundChannel
    {
    public:
        SoundChannel(int nbSamples);
        void writeSample( int time, short sample );
    private:
        std::vector< short > _samples;
    };

    class PAPU
    {
    public:
        PAPU( const Clock& clock );
        void writeByte( unsigned short addr, unsigned char value );
        unsigned char readByte( unsigned short addr ) const;
        void emulate( int nbCycles );
        const SoundChannel& getSoundMix() const;
    private:
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
        public:
            unsigned char _consecutive : 1;
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
            float getSoundLength()
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
            void writeByte( unsigned short addr, unsigned char value );
            unsigned char readByte( unsigned short addr ) const;
            void emulate( int nbCycles );
            const SoundChannel& getSoundChannel() const;
        private:
            float computeSample(float frequency, int timeSinceNoteStart) const;
            short getGbNote() const;
            bool isMuted() const;
            SoundChannel                                           _channel;
            Register< SoundLengthWavePatternDutyBits, 0xB0, 0xFF > _nr11;
            Register< EnveloppeBits >                              _nr12;
            Register< FrequencyLoBits, 0x0, 0xFF >                 _nr13;
            Register< FrequencyHiBits, 0x40, 0XFF >                _nr14;
            int                                                    _waveStart;
            int                                                    _waveLength;
            int                                                    _waveFrequency;
            const Clock&                                           _clock;
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
