#include <audio/envelope.h>
#include <base/cyclicCounter.imp.h>

namespace gbemu {

Envelope::Envelope(
    const unsigned short addr
) : _envelopeRegisterAddr( addr )
{}

void Envelope::clockEnvelope()
{
    // If envelope clock just clocked and we have a envelope going on.
    if (_rEnvelope.bits.sweepLength != 0) {
        // Increment the volume timer and if it overflowed...
        if (_volumeTimer.increment()) {
            // Update the volume.
            _volume += _rEnvelope.bits.direction();
            // Volume should never go above or under these values.
            clamp(_volume, 0, 15);
        }
    }
}

bool Envelope::writeByte( unsigned short addr, unsigned char value )
{
    if ( addr == _envelopeRegisterAddr ) {
        _rEnvelope.write( value );
        // JFX_LOG("-----NR12-ff12-----");
        // JFX_LOG("Initial channel volume       : " << (int)_rEnvelope.bits.initialVolume);
        // JFX_LOG("Volume sweep direction       : " << ( _rEnvelope.bits.isAmplifying() ? "up" : "down" ));
        // JFX_LOG("Length of each step          : " << _rEnvelope.bits.getSweepLength() << " seconds");
        _volumeTimer = CyclicCounter(0, _rEnvelope.bits.sweepLength);
        // Channel volume is reloaded from NR12.
        _volume = _rEnvelope.bits.initialVolume;
        return true;
    }
    return false;
}

}
