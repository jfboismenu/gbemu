#pragma once

#include <base/cyclicCounter.h>
#include <common/register.h>

namespace gbemu {

    class EnvelopeBits
    {
    public:
        char direction() const
        {
            return _direction == 1 ? 1: -1;
        }

        unsigned char sweepLength : 3;
    private:
        unsigned char _direction : 1;
    public:
        unsigned char initialVolume : 4;
    };

    class Envelope
    {
    public:
        Envelope(const unsigned short envelopeRegisterAddr);
        void clockEnvelope();
        bool writeByte( unsigned short addr, unsigned char value );

    protected:
        // Current volume.
        char _volume;

        Register< EnvelopeBits >        _rEnvelope;
        const unsigned short _envelopeRegisterAddr;
        CyclicCounter _volumeTimer;
    };
}
