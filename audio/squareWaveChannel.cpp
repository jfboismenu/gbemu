#include <audio/squareWaveChannel.h>
#include <audio/papu.h>
#include <base/cyclicCounter.imp.h>
#include <base/clock.h>
#include <base/logger.h>

namespace gbemu {

SquareWaveChannel::SquareWaveChannel(
    const PAPUClocks& clocks,
    std::mutex& mutex,
    unsigned short frequencyShiftRegisterAddr,
    unsigned short soundLengthRegisterAddr,
    unsigned short envelopeRegisterAddr,
    unsigned short frequencyLowRegisterAddr,
    unsigned short frequencyHiRegisterAddr
) :
    ChannelBase(clocks, mutex),
    Envelope(envelopeRegisterAddr),
    _frequencyTimer(0, 131000),
    _currentDutyStep(0),
    _frequencySweepRegisterAddr(frequencyShiftRegisterAddr),
    _soundLengthRegisterAddr(soundLengthRegisterAddr),
    _frequencyLowRegisterAddr(frequencyLowRegisterAddr),
    _frequencyHiRegisterAddr(frequencyHiRegisterAddr)
{}

bool SquareWaveChannel::contains(unsigned short addr) const
{
    return addr == _frequencySweepRegisterAddr ||
        addr == _soundLengthRegisterAddr ||
        addr == _envelopeRegisterAddr ||
        addr == _frequencyLowRegisterAddr ||
        addr == _frequencyHiRegisterAddr;
}

void SquareWaveChannel::writeByte(
    const unsigned short addr,
    const unsigned char value
)
{
    if ( addr == _frequencySweepRegisterAddr) {
        _rFrequencySweep.write(value);
        // JFX_LOG("-----NR10-ff10-----")
        // JFX_LOG("Increasing: " << _rFrequencySweep.bits.isIncreasing());
        // JFX_LOG("Number of shifts: " << _rFrequencySweep.bits.getNbSweepShifts());
        // JFX_LOG("Shift length: " << _rFrequencySweep.bits.getSweepLength() << " seconds");
    }
    else if ( addr == _soundLengthRegisterAddr ) {
        _rLengthDuty.write( value );

        _duty = _rLengthDuty.bits.wavePatternDuty();
        JFX_LOG("-----NR11-ff11-----");
        JFX_LOG("Wave pattern duty            : " << _rLengthDuty.bits.wavePatternDuty());
        JFX_LOG("Length counter load register : " << (int)_rLengthDuty.bits.soundLength);
    }
    else if ( Envelope::writeByte( addr, value ) ) {
        // Envelope was updated, nothing to do.
    }
    else if ( addr == _frequencyLowRegisterAddr ) {
        _rFrequencyLo.write( value );

        _frequencyPeriod = (2048 - getGbNote()) * 4;
        JFX_LOG("-----NR13-ff13-----");
        JFX_LOG("Frequency lo: " << (int)_rFrequencyLo.bits.freqLo);
    }
    else if ( addr == _frequencyHiRegisterAddr ) {
        _rFrequencyHiPlayback.write(value);
        JFX_LOG("-----NR14-" << std::hex << _frequencyHiRegisterAddr << std::dec << "-----");
        JFX_LOG("Frequency hi : " << (int)_rFrequencyHiPlayback.bits.freqHi);
        JFX_LOG("Consecutive  : " << ( _rFrequencyHiPlayback.bits.isLooping() ? "loop" : "play until NR11-length expires" ));
        JFX_LOG("Initialize?  : " << ( _rFrequencyHiPlayback.bits.initialize == 1 ));

        _frequencyPeriod = (2048 - getGbNote()) * 4;

        if ( _rFrequencyHiPlayback.bits.initialize ) {
            // FIXME: Enable channel bit.
            // FIXME: Set the sound length counter.
            // Reload period counter with frequency period.
            _frequencyTimer = CyclicCounter(_frequencyPeriod - 1, _frequencyPeriod);
            // Don't reset the step counter!!
            // FIXME: Volume envelope timer is reloaded with period.
            _volumeTimer = CyclicCounter(0, _rEnvelope.bits.sweepLength);
            // Channel volume is reloaded from NR12.
            _volume = _rEnvelope.bits.initialVolume;
            // FIXME: Noise channel's LFSR bits are all set to 1.
            // FIXME: Wave channel's position is set to 0 but sample buffer is NOT refilled.
            // FIXME: Square 1's sweep does several things (see frequency sweep).
        }
    }
}

void SquareWaveChannel::emulate(int currentCycle)
{
    if (_frequencyTimer.getCycleLength() == 0) {
        return;
    }

    // If frequency timer didn't underflow, output doesn't change.
    if (!_frequencyTimer.decrement()) {
        return;
    }
    _frequencyTimer = CyclicCounter(_frequencyPeriod, _frequencyPeriod);
    _currentDutyStep.increment();
    //std::cout << _currentDutyStep.count() << " " << _duty << std::endl;
    insertEvent(
        currentCycle,
        _currentDutyStep < _duty ? -_volume : _volume
    );
}

short SquareWaveChannel::getGbNote() const
{
    return _rFrequencyLo.bits.freqLo | ( _rFrequencyHiPlayback.bits.freqHi << 8 );
}

}
