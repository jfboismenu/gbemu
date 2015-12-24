#pragma once

#include <audio/squareWaveChannel.h>
#include <common/register.h>
#include <mutex>

namespace gbemu {

    class Clock;

    class PAPU
    {
    public:
        static void renderAudio(void* output, const unsigned long frameCount, const int rate, void* userData);
        PAPU( const Clock& clock );
        void writeByte( unsigned short addr, unsigned char value );
        unsigned char readByte( unsigned short addr ) const;

        float getCurrentPlaybackTime() const;
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

        SquareWaveChannel  _squareWaveChannel1;
        SquareWaveChannel  _squareWaveChannel2;

        /* To be refactored when we address the noise and wave channel
        class SoundLengthBits : private SoundLengthWavePatternDutyBits
        {
        public:
            unsigned char getSoundLength()
            {
                return SoundLengthWavePatternDutyBits::getSoundLength();
            }
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

        Register< EnveloppeBits > _nr32;
        Register< SoundLengthBits, 0xB0, 0xFF > _nr41;
        Register< NoiseEnveloppeBits > _nr42;*/

        Register< MainVolumeOutputControlBits > _nr50;
        Register< SoundOutputTerminalSelect > _nr51;
        Register< NR52bits, 0xFF, 0xF0 > _nr52;

        int _rate;
        int64_t _currentPlaybackTime;
        const Clock& _clock;
        bool                      _initializing;
    };
}
