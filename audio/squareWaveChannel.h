#pragma once

#include <audio/channelBase.h>
#include <audio/common.h>
#include <audio/envelope.h>
#include <audio/frequency.h>
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
                    JFX_MSG_ABORT( "Wave pattern duty invalid: " << _wavePatternDuty );
            }
        }
        unsigned char soundLength : 6;
    private:
        unsigned char _wavePatternDuty : 2;
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

    class SquareWaveChannel : public ChannelBase, public Envelope, public Frequency
    {
    public:
        SquareWaveChannel(
            const PAPUClocks& clock,
            std::mutex& mutex,
            unsigned short frequencyShiftRegisterAddr,
            unsigned short soundLengthRegisterAddr,
            unsigned short envelopeRegisterAddr,
            unsigned short frequencyLowRegisterAddr,
            unsigned short frequencyHiRegisterAddr
        );
        bool contains(unsigned short addr) const;
        void writeByte( unsigned short addr, unsigned char value );
        unsigned char readByte( unsigned short addr ) const;
        void emulate(int64_t cycle);
    private:
        short getGbNote() const;

        Register< FrequencySweepBits, 0xFF, 0xFF >             _rFrequencySweep;
        Register< SoundLengthWavePatternDutyBits, 0xB0, 0xFF > _rLengthDuty;

        // Current step in the played frequency.
        CyclicCounterT<8> _currentDutyStep;
        int _duty;

        const unsigned short _frequencySweepRegisterAddr;
        const unsigned short _soundLengthRegisterAddr;
    };
}
