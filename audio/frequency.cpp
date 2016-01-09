#include <audio/frequency.h>
#include <base/cyclicCounter.imp.h>
#include <base/logger.h>

namespace gbemu {

Frequency::Frequency(
    unsigned short frequencyLowRegisterAddr,
    unsigned short frequencyHiRegisterAddr
) : _frequencyLowRegisterAddr(frequencyLowRegisterAddr),
    _frequencyHiRegisterAddr(frequencyHiRegisterAddr),
    _frequencyTimer(0, 131000)
{}

bool Frequency::writeByte(
    const unsigned short addr,
    const unsigned char value
)
{
    if ( addr == _frequencyLowRegisterAddr ) {
        _rFrequencyLo.write( value );

        _frequencyPeriod = (2048 - getGbNote()) * 4;
        JFX_LOG("-----NR13-ff13-----");
        JFX_LOG("Frequency lo: " << (int)_rFrequencyLo.bits.freqLo);
        return true;
    }
    else if ( addr == _frequencyHiRegisterAddr ) {
        _rFrequencyHiPlayback.write(value);
        JFX_LOG("-----NR14-" << std::hex << _frequencyHiRegisterAddr << std::dec << "-----");
        JFX_LOG("Frequency hi : " << (int)_rFrequencyHiPlayback.bits.freqHi);
        JFX_LOG("Consecutive  : " << ( _rFrequencyHiPlayback.bits.isLooping() ? "loop" : "play until NR11-length expires" ));
        JFX_LOG("Initialize?  : " << ( _rFrequencyHiPlayback.bits.initialize == 1 ));

        _frequencyPeriod = (2048 - getGbNote()) * 4;

        if ( _rFrequencyHiPlayback.bits.initialize ) {
            // Reload period counter with frequency period.
            _frequencyTimer = CyclicCounter(_frequencyPeriod - 1, _frequencyPeriod);
        }
        return true;
    }
    return false;
}


bool Frequency::emulate(int currentCycle)
{

    if (_frequencyTimer.getCycleLength() == 0) {
        return false;
    }

    // If frequency timer didn't underflow, output doesn't change.
    if (!_frequencyTimer.decrement()) {
        return false;
    }
    _frequencyTimer = CyclicCounter(_frequencyPeriod, _frequencyPeriod);
    return true;
}

short Frequency::getGbNote() const
{
    return _rFrequencyLo.bits.freqLo | ( _rFrequencyHiPlayback.bits.freqHi << 8 );
}

}
