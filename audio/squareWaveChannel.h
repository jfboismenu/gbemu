#pragma once

#include <audio/channelBase.h>
#include <audio/common.h>
#include <common/register.h>

namespace gbemu {

    class CPUClock;
    class PAPUClocks;

    class SoundLengthWavePatternDutyBits
    {
    public:
        int wavePatternDuty() const
        {
            switch( _wavePatternDuty ) {
                case 0:
                    return 1;
                case 1:
                    return 2;
                case 2:
                    return 4;
                case 3:
                    return 6;
                default:
                    JFX_MSG_ASSERT( "Wave pattern duty invalid: " << _wavePatternDuty );
            }
        }
        unsigned char soundLength : 6;
    private:
        unsigned char _wavePatternDuty : 2;
    };

    class EnveloppeBits
    {
    public:
        char getVolumeDelta() const
        {
            return _direction == 1 ? 1: -1;
        }

        unsigned char sweepLength : 3;
    private:
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

    class SquareWaveChannel : public ChannelBase
    {
    public:
        SquareWaveChannel(
            const PAPUClocks& clock,
            std::mutex& mutex,
            unsigned short frequencyShiftRegisterAddr,
            unsigned short soundLengthRegisterAddr,
            unsigned short evenloppeRegisterAddr,
            unsigned short frequencyLowRegisterAddr,
            unsigned short frequencyHiRegisterAddr
        );
        bool contains(unsigned short addr) const;
        void writeByte( unsigned short addr, unsigned char value );
        unsigned char readByte( unsigned short addr ) const;
        void emulate(int cycle);
        void clockEnveloppe();
    private:
        short getGbNote() const;

        Register< FrequencySweepBits, 0xFF, 0xFF >             _rFrequencySweep;
        Register< SoundLengthWavePatternDutyBits, 0xB0, 0xFF > _rLengthDuty;
        Register< EnveloppeBits >                              _rEnveloppe;
        Register< FrequencyLoBits, 0x0, 0xFF >                 _rFrequencyLo;
        Register< FrequencyHiBits, 0x40, 0XFF >                _rFrequencyHiPlayback;

        // Length of the period for each of the current frequency's step.
        // A frequency has 8 cycles of _frequency_period length.
        int _frequency_period;
        // Current volume.
        char _volume;
        // How far we're into the current period.
        CyclicCounter _frequency_timer;
        // Current step in the played frequency.
        CyclicCounterT<8> _current_duty_step;
        CyclicCounter _volumeTimer;
        int _duty;
        char _last_sample;

        const unsigned short _frequencySweepRegisterAddr;
        const unsigned short _soundLengthRegisterAddr;
        const unsigned short _evenloppeRegisterAddr;
        const unsigned short _frequencyLowRegisterAddr;
        const unsigned short _frequencyHiRegisterAddr;
    };
}
