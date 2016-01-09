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
    Frequency(
        frequencyLowRegisterAddr,
        frequencyHiRegisterAddr
    ),
    _currentDutyStep(0),
    _frequencySweepRegisterAddr(frequencyShiftRegisterAddr),
    _soundLengthRegisterAddr(soundLengthRegisterAddr)
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
    else if ( Frequency::writeByte( addr, value ) && addr == _frequencyHiRegisterAddr ) {
        if ( _rFrequencyHiPlayback.bits.initialize ) {
            // FIXME: Enable channel bit.
            // FIXME: Set the sound length counter.
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
    // If the frequency clock hasn't overflowed
    if ( !Frequency::emulate(currentCycle) ) {
        return;
    }
    _currentDutyStep.increment();
    //std::cout << _currentDutyStep.count() << " " << _duty << std::endl;
    insertEvent(
        currentCycle,
        _currentDutyStep < _duty ? -_volume : _volume
    );
}


}
