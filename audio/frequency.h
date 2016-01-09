#pragma once

#include <base/cyclicCounter.h>
#include <common/register.h>

namespace gbemu {

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

    class Frequency
    {
    public:
        Frequency(
            const unsigned short frequencyLowRegisterAddr,
            const unsigned short frequencyHiRegisterAddr
        );
        bool writeByte( unsigned short addr, unsigned char value );
        unsigned char readByte( unsigned short addr ) const;
        bool emulate(int cycle);
    protected:
        short getGbNote() const;

        Register< FrequencyLoBits, 0x0, 0xFF >                 _rFrequencyLo;
        Register< FrequencyHiBits, 0x40, 0XFF >                _rFrequencyHiPlayback;

        // Length of the period for each of the current frequency's step.
        // A frequency has 8 cycles of _frequencyPeriod length.
        int _frequencyPeriod;
        // How far we're into the current period.
        CyclicCounter _frequencyTimer;

        const unsigned short _frequencyLowRegisterAddr;
        const unsigned short _frequencyHiRegisterAddr;
    };
}
